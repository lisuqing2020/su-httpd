src=$(wildcard ./src/*.c ./lib/*.c)
inc=include
out=builds/
std=c99

httpd:$(src)
	$(CC) $(src) -I $(inc) -o $(out)$@ -std=$(std)

.PHONY:install
install:
	cp ./builds/httpd /usr/bin
