
print -n "write file, no explicitly msync/munmap/close it: \t"
./no_close $1 $2
cmp $1 $2
print $?
