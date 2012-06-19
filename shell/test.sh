# perform 5 tests:
# 
# 1.function test:
# map -> read/write memory -> munmap
# and there are 2 kinds of read/write memory:
# 1)continuous read/write
# 2)random read/write
# 
# 2.protection test:
# open file with (O_READ|O_WRITE|O_RDWR) -> 
# map file with (PROT_READ|PROT_WRITE,MAP_SHARED|MAP_PRIVATE)
#
# 3.special test:
# 1)write into the mapped file after close it,
# and check if the change synced to it.
# 2)write into the mapped file, dont munmap or close it,
# and check if the change synced to it.
#
# 4.consistent test:(not yet)
# test if file i/o is consistent with mmap in smbfs:
# read/write file -> map -> read/write memory -> read/write file -> munmap
#
# 5.extend test:(not yet)
# access beyond file size:
# 1)read/write memory beyond file size
# 2)read/write file beyond file size


# how to test:
# 1. make
# 2. make install
# 3. cd shell 
# 4. ksh93 test.sh smbfs_directory zfs_directory

if [[ $# != 2 ]]
then print "usage: test.sh smbfs_directory zfs_directory"
exit -1
fi

testfile=("/testfile0" "/testfile1" "/testfile2" "/testfile3")

#for ((i=0; i<3; i++))
#do

tf1=$1${testfile[0]}
tf2=$2${testfile[0]}
tf3=$1${testfile[1]}
tf4=$2${testfile[1]}
tf5=$1${testfile[2]}
tf6=$2${testfile[2]}
tf7=$1${testfile[3]}


print "\nbegin test"

print "\nfunction test"
#continuous write
. ./mmap_test0.sh $tf1 $tf2
sleep 2
#discrete write
. ./mmap_test1.sh $tf3 $tf4
sleep 2
#continuous read
. ./mmap_test2.sh $tf3 $tf4
sleep 2
#discrete read
. ./mmap_test3.sh $tf1 $tf2
sleep 2

print "\nprotection test"
#O_RDONLY, PROT_READ|PROT_WRITE, MAP_SHARED
. ./mmap_test4.sh $tf1 $tf2
sleep 2
#O_RDWR, PROT_READ|PROT_WRITE, MAP_SHARED
. ./mmap_test5.sh $tf1 $tf2
sleep 2
#O_RDWR, PROT_READ|PROT_WRITE, MAP_PRIVATE
. ./mmap_test6.sh $tf1 $tf2
sleep 2

print "\nspecial test"
#write file after close it
. ./mmap_test7.sh $tf5 $tf6
sleep 2
#write file, no explicitly msync/munmap/close it
. ./mmap_test8.sh $tf5 $tf6
sleep 2
#test a big file(8GB)
#. ./mmap_test9.sh 6 $tf7

#print "\nclean..."
#rm -rf $tf0 $tf2 $tf4
#rm -rf $tf1 $tf3 $tf5 $tf7

print "\n all done!"
#done
