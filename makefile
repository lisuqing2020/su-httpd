src=$(wildcard ./src/*.c)
inc=include
out=builds/

httpd:$(src)
	$(CC) $(src) -I $(inc) -o $(out)$@
