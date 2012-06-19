
print -n "O_RDWR\tPROT_READ|PROT_WRITE\tMAP_SHARED\t"
./rd_mmap1 -o rw -m rws -f $1 $2
print $?
