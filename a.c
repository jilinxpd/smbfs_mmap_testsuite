#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

/* we assume that the test file in smbfs is the same as
 * the test file in local fs */

/* continuous read */
int main(int argc, char *argv[]) {

    struct stat sb;
    char *smbfs_file, *local_file;
    char *smbfs_addr, *local_addr;
    off_t offset;
    size_t len;
    size_t filesize;
    int smbfs_fid, local_fid;
    int ret, mret = 0;
    int flags = 0, mflags = 0, prot = 0;
    int i, j;

    /* parse arguments */
    if (argc != 8) {
        printf("\tusage:\n\ta -o r|w -m r|w|s|p -f file1 file2\n");
        return -1;
    }
    for (i = 1; i < argc;) {
        switch (argv[i][1]) {
            case 'o':/* options for open() */
                i++;
                for (j = 0; argv[i][j]; j++) {
                    if (argv[i][j] == 'r')
                        flags |= O_RDONLY;
                    else if (argv[i][j] == 'w')
                        flags |= O_WRONLY;
                }
                if (flags & (O_RDONLY | O_WRONLY) == (O_RDONLY | O_WRONLY))
                    flags = O_RDWR;
                i++;
                break;
            case 'm':/* options for mmap() */
                i++;
                for (j = 0; argv[i][j]; j++) {
                    if (argv[i][j] == 'r')
                        prot |= PROT_READ;
                    else if (argv[i][j] == 'w')
                        prot |= PROT_WRITE;
                    else if (argv[i][j] == 's')
                        mflags |= MAP_SHARED;
                    else if (argv[i][j] == 'p')
                        mflags |= MAP_PRIVATE;
                }
                i++;
                break;
            case 'f':/* target files */
                i++;
                smbfs_file = argv[i];
                i++;
                local_file = argv[i];
                i++;
        }
    }

    /* test file, should in smbfs */
    smbfs_fid = open(smbfs_file, flags);
    if (smbfs_fid == -1) {
        printf("open %s error=%d\n", smbfs_file, errno);
        mret = -2;
        goto exit5;
    }
    /* another test file, should in local fs such as zfs */
    local_fid = open(local_file, flags);
    if (local_fid == -1) {
        printf("open %s error=%d\n", local_file, errno);
        mret = -3;
        goto exit4;
    }

    /* get file size */
    if (fstat(smbfs_fid, &sb) == -1) {
        printf("fstat %s error=%d\n", smbfs_file, errno);
        mret = -4;
        goto exit3;
    }
    filesize = sb.st_size;

    /* map file */
    offset = 0;
    len = filesize;
    smbfs_addr = mmap(NULL, len, prot, mflags, smbfs_fid, offset);
    if (smbfs_addr == MAP_FAILED) {
        printf("mmap %s error=%d\n", smbfs_file, errno);
        mret = -5;
        goto exit3;
    }
    local_addr = mmap(NULL, len, prot, mflags, local_fid, offset);
    if (local_addr == MAP_FAILED) {
        printf("mmap %s error=%d\n", local_file, errno);
        mret = -6;
        goto exit2;
    }

    /* compare the memory regions */
    len = filesize;
    ret = memcmp(smbfs_addr, local_addr, len);
    if (ret != 0) {
        printf("memcmp %s %c %s\n", smbfs_file, ret > 0 ? '>' : '<', local_file);
        mret = -7;
        goto exit1;
    }

    /* unmap file */
exit1:
    len = filesize;
    ret = munmap(local_addr, len);
    if (ret == -1) {
        printf("munmap %s error=%d\n", local_file, errno);
    }
exit2:
    len = filesize;
    ret = munmap(smbfs_addr, len);
    if (ret == -1) {
        printf("munmap %s error=%d\n", smbfs_file, errno);
    }

    /* close file */
exit3:
    close(local_fid);
exit4:
    close(smbfs_fid);
exit5:
    return mret;
}
