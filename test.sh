# test files with different size respectively:
# 9K
# 12K
# 15K

# perform 4 tests: 
# 
# 1.protection test:
# open file with (O_READ|O_WRITE|O_RDWR) -> 
# map file with (PROT_READ|PROT_WRITE,MAP_SHARED|MAP_PRIVATE)
# 
# 2.function test:
# map -> read/write memory -> munmap
# and there are 2 kinds of read/write memory:
# 1)continuous read/write
# 2)random read/write
#
# 3.compound test:
# read/write file -> map -> read/write memory -> read/write file -> munmap
#
# 4.extend test:(not yet)
# access beyond file size:
# 1)read/write memory beyond file size
# 2)read/write file beyond file size


# how to test:
# 1. put the same test files in smbfs_directory and zfs_directory.
# 2. cd testsuites 
# 3. run this shell: test.sh smbfs_directory zfs_directory

if [[ $# != 2 ]]
then print "usage: test.sh smbfs_directory zfs_directory"
exit -1
fi

testfile=("/testfile_9k" "/testfile_12k" "/testfile_15k")

for ((i=0; i<3; i++))
do

tf1=$1${testfile[i]}
tf2=$2${testfile[i]}

print "\ntesting "$tf1" and "$tf2

print "\nprotection test"
print -n "O_RDONLY\tPROT_READ|PROT_WRITE\tMAP_SHARED\t"
./a -o r -m rws -f $tf1 $tf2
print $?
sleep 2
print -n "O_RDWR\tPROT_READ|PROT_WRITE\tMAP_SHARED\t"
./a -o rw -m rws -f $tf1 $tf2
print $?
sleep 2
print -n "O_RDWR\tPROT_READ|PROT_WRITE\tMAP_PRIVATE\t"
./a -o rw -m rwp -f $tf1 $tf2
print $?
sleep 2

print "\nfunction test"
print -n "continuous read: \t"
./a -o r -m rs -f $tf1 $tf2
print $?
sleep 2
print -n "random read: \t"
./b $tf1 $tf2
print $?
sleep 2
print -n "continuous write: \t"
./c $tf1 $tf2
sleep 2
cmp $tf1 $tf2
print $?
sleep 1
print -n "random write: \t"
./d $tf1 $tf2
sleep 2
cmp $tf1 $tf2
print $?
sleep 1

done
