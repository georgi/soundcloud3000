LIBS := -ljansson -lcurl -lportaudio -lmpg123 
LIBDIR := -Lmpg123/lib
INCLUDE := -Impg123/include -Itermbox/src
OBJECTS := termbox/build/src/libtermbox.a
OUTPUT := soundcloud3000

build:
	gcc *.c $(LIBS) $(OBJECTS) $(INCLUDE) $(LIBDIR) -o $(OUTPUT)

lib:
	cd mpg123 && ./configure $(pwd) && make && make install

