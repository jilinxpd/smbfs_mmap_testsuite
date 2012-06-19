
print -n "test a big file(8GB), takes a bit long time: \t"
# $1 stride
# $2 testfile
./mmaptest $1 $2
print $?
