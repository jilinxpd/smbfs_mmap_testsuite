
print -n "continuous read: \t"
./rd_mmap1 -o r -m rs -f $1 $2
print $?
