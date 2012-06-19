targets=rd_mmap1 rd_mmap2 wr_mmap1 wr_mmap2 close_wr no_close mmaptest

all:
	(cd ./src && make)

install:
	@(for target in $(targets); do (cp ./src/$$target ./shell); done;)

uninstall: clean
	@(for target in $(targets); do (rm ./shell/$$target); done;)
	
clean:
	(cd ./src && make clean)
