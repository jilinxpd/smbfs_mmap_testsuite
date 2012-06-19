smbfs_mmap_testsuite
====================

a simple test suite for smbfs-mmap.

perform 5 tests:

1.function test:
map -> read/write memory -> munmap
and there are 2 kinds of read/write memory:
1)continuous read/write
2)random read/write

2.protection test:
open file with (O_READ|O_WRITE|O_RDWR) -> 
map file with (PROT_READ|PROT_WRITE,MAP_SHARED|MAP_PRIVATE)

3.special test:
1)write into the mapped file after close it,
and check if the change synced to it.
2)write into the mapped file, dont munmap or close it,
and check if the change synced to it.

4.consistent test:(not yet)
test if file i/o is consistent with mmap in smbfs:
read/write file -> map -> read/write memory -> read/write file -> munmap

5.extend test:(not yet)
access beyond file size:
1)read/write memory beyond file size
2)read/write file beyond file size


how to test:
1. make
2. make install
3. cd shell 
4. ksh93 test.sh smbfs_directory zfs_directory
