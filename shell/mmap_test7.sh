
print -n "write file after close it: \t"
./close_wr $1 $2
cmp $1 $2
print $?
