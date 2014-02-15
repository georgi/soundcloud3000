LIB := lib/libmpg123.la lib/libtermbox.a
OUTPUT := soundcloud3000
PWD := $(shell pwd)
OBJ := api.o main.o stream.o lib/libmpg123.a lib/libtermbox.a lib/libjansson.a http-parser/libhttp_parser.o sds/sds.o hitpoint/hitpoint.o hitman/hitman.o

server: http-parser/libhttp_parser.o server.c
	gcc -g -Wall -lportaudio server.c -Llib -o server

build: $(LIB) $(OBJ)
	gcc -lportaudio $(OBJ) -Llib -o $(OUTPUT)

%.o: %.c
	gcc -g -Wall -Iinclude -c $< -o $@

hitpoint/hitpoint.o: hitpoint/hitpoint.c
	cd hitpoint && make hitpoint.o

hitman/hitman.o: hitman/hitman.c
	cd hitman && make hitman.o

lib/libportaudio.a:
	cd portaudio && ./configure --enable-static=yes --prefix=$(PWD) && make && make install

lib/libjansson.a:
	cd jansson && ./configure --enable-static=yes --prefix=$(PWD) && make && make install

lib/libmpg123.a:
	cd mpg123 && ./configure --enable-static=yes --prefix=$(PWD) && make && make install

lib/libtermbox.a:
	cd termbox && ./waf configure --prefix=$(PWD) && ./waf && ./waf install

http-parser/libhttp_parser.o:
	cd http-parser && make libhttp_parser.o
