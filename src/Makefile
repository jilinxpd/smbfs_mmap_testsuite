targets=rd_mmap1 rd_mmap2 wr_mmap1 wr_mmap2 close_wr no_close mmaptest

all: $(targets)

clean:
	rm -rf $(targets)

$(targets): %: %.c
	gcc -o $@ $<
