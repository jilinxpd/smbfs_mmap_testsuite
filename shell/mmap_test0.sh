
print -n "continuous write: \t"
./wr_mmap1 $1 $2
cmp $1 $2
print $?
