# Compiler
CC = g++

# Include paths
INCLUDES = -I. -I/usr/local/include/OGRE/ -I/usr/local/include/

# Library paths
LIBS = -L. -L/usr/local/lib -lm

arucocards: TinyOgre.o
	$(CC) TinyOgre.o -l:libaruco.so.1.2.4 -lOgreMain $(LIBS) -o arucocards

TinyOgre.o: TinyOgre.cpp
	$(CC) $(INCLUDES) -c TinyOgre.cpp

clean:
	rm TinyOgre.o arucocards
