
print -n "O_RDWR\tPROT_READ|PROT_WRITE\tMAP_PRIVATE\t"
./rd_mmap1 -o rw -m rwp -f $1 $2
print $?
