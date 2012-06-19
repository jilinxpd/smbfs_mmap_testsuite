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
 * the one in zfs */

/* test if smbfs-mmap can handle discrete read
 * correctly, discrete read will possibly escape
 * block i/o, so actually we are testing getpage
 * without block i/o.*/

int main(int argc, char *argv[]) {

    struct stat sb;
    char *smbfs_addr, *local_addr;
    off_t offset;
    size_t filesize;
    size_t blksize;
    long numblks;
    int smbfs_fid, local_fid;
    int mret = 0;
    int i;

    blksize = sysconf(_SC_PAGESIZE);
    if (blksize < 0) {
        printf("sysconf error=%d\n", errno);
        mret = -8;
        goto exit4;
    }

    /* test file, should in smbfs */
    smbfs_fid = open(argv[1], O_RDONLY);
    if (smbfs_fid == -1) {
        printf("open %s error=%d\n", argv[1], errno);
        mret = -1;
        goto exit4;
    }
    /* another test file, should in local fs such as zfs */
    local_fid = open(argv[2], O_RDONLY);
    if (local_fid == -1) {
        printf("open %s error=%d\n", argv[2], errno);
        mret = -2;
        goto exit3;
    }

    /* get file size */
    if (fstat(smbfs_fid, &sb) == -1) {
        printf("fstat %s error=%d\n", argv[1], errno);
        mret = -6;
        goto exit2;
    }
    filesize = sb.st_size;

    /* map one page each time and compare the content.
     * note that we do discrete map.
     */
    numblks = filesize / blksize;
    for (i = 0; i < 3 * numblks && mret >= 0; i += 3) {

        /* map file */
        offset = (i % numblks) * blksize;
        smbfs_addr = mmap(NULL, blksize, PROT_READ, MAP_SHARED, smbfs_fid, offset);
        if (smbfs_addr == MAP_FAILED) {
            printf("mmap %ld of %s error=%d\n", offset, argv[1], errno);
            mret = -3;
            continue;
        }
        local_addr = mmap(NULL, blksize, PROT_READ, MAP_SHARED, local_fid, offset);
        if (local_addr == MAP_FAILED) {
            printf("mmap %ld of %s error=%d\n", offset, argv[2], errno);
            mret = -4;
            goto exit1;
        }

        /* compare the memory regions */
        if (memcmp(smbfs_addr, local_addr, blksize) != 0) {
            printf("memcmp not equal: %d\n", i);
            mret = -5;
        }

        /* unmap file */
        if (munmap(local_addr, blksize) == -1) {
            printf("munmap %ld of %s error=%d\n", offset, argv[2], errno);
            mret = -13;
        }
exit1:
        if (munmap(smbfs_addr, blksize) == -1) {
            printf("munmap %ld of %s error=%d\n", offset, argv[1], errno);
            mret = -14;
        }
    }

    /* close file */
exit2:
    close(local_fid);
exit3:
    close(smbfs_fid);
exit4:
    return mret;
}
