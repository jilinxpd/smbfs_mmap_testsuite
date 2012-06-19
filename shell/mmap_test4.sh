
print -n "O_RDONLY\tPROT_READ|PROT_WRITE\tMAP_SHARED\t"
./rd_mmap1 -o r -m rws -f $1 $2
print $?
