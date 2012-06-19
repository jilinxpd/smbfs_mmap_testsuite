
print -n "discrete write: \t"
./wr_mmap2 $1 $2
cmp $1 $2
print $?
