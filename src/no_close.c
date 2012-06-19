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

/* It doesn't matter if the userland forgets to close the file or
 * munmap it, because smbfs will help close it(including otW close)
 * and sync the dirty pages to the file.
 * This program tests if smbfs works as we said. */

int main(int argc, char *argv[]) {

    char *smbfs_addr, *local_addr;
    off_t offset;
    size_t filesize;
    size_t blksize;
    int smbfs_fid, local_fid;
    int mret = 0;
    int i;
    char *c = "?#*%&";

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
        goto exit5;
    }

    /* extend file */
    filesize = 64 * 1024;
    if (ftruncate(smbfs_fid, filesize) == -1) {
        printf("ftrunc %s error=%d\n", argv[1], errno);
        mret = -11;
        goto exit5;
    }
    if (ftruncate(local_fid, filesize) == -1) {
        printf("ftrunc %s error=%d\n", argv[2], errno);
        mret = -12;
        goto exit5;
    }

    /* map file */
    smbfs_addr = mmap(NULL, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, smbfs_fid, 0);
    if (smbfs_addr == MAP_FAILED) {
        printf("mmap %s error=%d\n", argv[1], errno);
        mret = -3;
        goto exit5;
    }
    local_addr = mmap(NULL, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, local_fid, 0);
    if (local_addr == MAP_FAILED) {
        printf("mmap %s error=%d\n", argv[2], errno);
        mret = -4;
        goto exit5;
    }

    /* continuously write into mapped addrs */
    blksize = filesize / 4;
    for (i = 0, offset = 0; i < 4; i++, offset += blksize) {
        memset(smbfs_addr + offset, c[i], blksize);
        memset(local_addr + offset, c[i], blksize);
    }
    memset(smbfs_addr + offset, c[i], filesize - offset);
    memset(local_addr + offset, c[i], filesize - offset);

    /* no msync, munmap, close */

exit5:
    return mret;
}
