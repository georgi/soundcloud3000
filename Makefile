soundcloud3000:
	gcc -ljansson -lcurl -lportaudio -lmpg123 *.c -Impg123/include -Lmpg123/lib -o soundcloud3000

lib:
	cd mpg123 && ./configure $(pwd) && make && make install

