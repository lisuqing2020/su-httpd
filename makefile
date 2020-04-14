src=$(wildcard ./src/*.c)
inc=include
out=builds/
std=c99

httpd:$(src)
	$(CC) $(src) -I $(inc) -o $(out)$@ -std=$(std)
