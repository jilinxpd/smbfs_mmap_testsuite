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

/* random read */
int main(int argc, char *argv[]) {

    struct stat sb;
    char *smbfs_addr, *local_addr;
    off_t offset;
    size_t len;
    size_t filesize;
    size_t blksize;
    int smbfs_fid, local_fid;
    int ret, mret = 0;
    int addr_offset[4];
    int i;

    /* test file, should in smbfs */
    smbfs_fid = open(argv[1], O_RDONLY);
    if (smbfs_fid == -1) {
        printf("open %s error=%d\n", argv[1], errno);
        mret = -1;
        goto exit5;
    }
    /* another test file, should in local fs such as zfs */
    local_fid = open(argv[2], O_RDONLY);
    if (local_fid == -1) {
        printf("open %s error=%d\n", argv[2], errno);
        mret = -2;
        goto exit4;
    }

    /* get file size */
    if (fstat(smbfs_fid, &sb) == -1) {
        printf("fstat %s error=%d\n", argv[1], errno);
        mret = -3;
        goto exit3;
    }
    filesize = sb.st_size;

    /* map file */
    offset = 0;
    len = filesize;
    smbfs_addr = mmap(NULL, len, PROT_READ, MAP_SHARED, smbfs_fid, offset);
    if (smbfs_addr == MAP_FAILED) {
        printf("mmap %s error=%d\n", argv[1], errno);
        mret = -4;
        goto exit3;
    }
    local_addr = mmap(NULL, len, PROT_READ, MAP_SHARED, local_fid, offset);
    if (local_addr == MAP_FAILED) {
        printf("mmap %s error=%d\n", argv[2], errno);
        mret = -5;
        goto exit2;
    }

    /* compare the memory regions */
    blksize = filesize / 4;
    addr_offset[0] = blksize * 2;
    addr_offset[1] = blksize * 0;
    addr_offset[2] = blksize * 1;
    addr_offset[3] = blksize * 3;
    for (i = 0; i < 4; i++) {
        ret = memcmp(smbfs_addr + addr_offset[i], local_addr + addr_offset[i], blksize);
        if (ret != 0) {
            printf("memcmp %s != %s\n", argv[1], argv[2]);
            mret = -6;
            goto exit1;
        }
    }

    /* unmap file */
exit1:
    len = filesize;
    ret = munmap(local_addr, len);
    if (ret == -1) {
        printf("munmap %s error=%d\n", argv[2], errno);
    }
exit2:
    len = filesize;
    ret = munmap(smbfs_addr, len);
    if (ret == -1) {
        printf("munmap %s error=%d\n", argv[1], errno);
    }

    /* close file */
exit3:
    close(local_fid);
exit4:
    close(smbfs_fid);
exit5:
    return mret;
}
