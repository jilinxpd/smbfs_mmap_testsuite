#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

/* we create 2 same tmp files in smbfs and zfs. */

/* test if smbfs-mmap can handle discrete write
 * correctly, discrete write will possibly escape
 * block i/o, so actually we are testing putpage
 * without block i/o. */

int main(int argc, char *argv[]) {

    char *smbfs_addr, *local_addr;
    off_t offset;
    size_t filesize;
    size_t blksize;
    long numblks;
    int smbfs_fid, local_fid;
    int mret = 0;
    int i;
    char *c = "?#*%&";

    blksize = sysconf(_SC_PAGESIZE);
    if (blksize < 0) {
        printf("sysconf error=%d\n", errno);
        mret = -8;
        goto exit5;
    }

    /* test file, should in smbfs */
    smbfs_fid = open(argv[1], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH);
    if (smbfs_fid == -1) {
        printf("open %s error=%d\n", argv[1], errno);
        mret = -1;
        goto exit5;
    }
    /* another test file, should in local fs such as zfs */
    local_fid = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH);
    if (local_fid == -1) {
        printf("open %s error=%d\n", argv[2], errno);
        mret = -2;
        goto exit4;
    }

    /* extend file */
    filesize = 1 * 1024 * 1024;
    if (ftruncate(smbfs_fid, filesize) == -1) {
        printf("ftrunc %s error=%d\n", argv[1], errno);
        mret = -11;
        goto exit3;
    }
    if (ftruncate(local_fid, filesize) == -1) {
        printf("ftrunc %s error=%d\n", argv[2], errno);
        mret = -12;
        goto exit3;
    }

    /* map one page each time and write to it.
     * note that we do discrete map.
     */
    numblks = filesize / blksize;
    for (i = 0; i < 3 * numblks && mret >= 0; i += 3) {

        /* map file */
        offset = (i % numblks) * blksize;
        smbfs_addr = mmap(NULL, blksize, PROT_READ | PROT_WRITE, MAP_SHARED, smbfs_fid, offset);
        if (smbfs_addr == MAP_FAILED) {
            printf("mmap %s error=%d\n", argv[1], errno);
            mret = -3;
            continue;
        }
        local_addr = mmap(NULL, blksize, PROT_READ | PROT_WRITE, MAP_SHARED, local_fid, offset);
        if (local_addr == MAP_FAILED) {
            printf("mmap %s error=%d\n", argv[2], errno);
            mret = -4;
            goto exit2;
        }

        /* write into mapped addrs */
        memset(smbfs_addr, c[i % 5], blksize);
        memset(local_addr, c[i % 5], blksize);

        /* sync mapped pages to file */
        if (msync(local_addr, blksize, MS_SYNC) == -1) {
            printf("munmap %s error=%d\n", argv[2], errno);
            mret = -9;
            goto exit1;
        }
        if (msync(smbfs_addr, blksize, MS_SYNC) == -1) {
            printf("munmap %s error=%d\n", argv[1], errno);
            mret = -10;
        }

        /* unmap file */
exit1:
        if (munmap(local_addr, blksize) == -1) {
            printf("munmap %s error=%d\n", argv[2], errno);
            mret = -13;
        }
exit2:
        if (munmap(smbfs_addr, blksize) == -1) {
            printf("munmap %s error=%d\n", argv[1], errno);
            mret = -14;
        }
    }

    /* close file */
exit3:
    close(local_fid);
exit4:
    close(smbfs_fid);
exit5:
    return mret;
}
