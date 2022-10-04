#CC =  x86_64-w64-mingw32-gcc
CC = gcc
CFLAGS = -Wall -I. -g -fstack-protector-all -D_FORTIFY_SOURCE=2
DEPS = system.h
OBJ = main.o map.o units.o items.o skills.o jsmn.o parse.o ai.o
LIB = ../lib/libtcod.so ../lib/libtcodxx.so

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

game: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIB) -Wl,-rpath=../lib
	
clean:
	/bin/rm -f $(OBJ)
