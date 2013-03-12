# Compiler
CC = g++

# Source
SRC = OgreARAppLogic.cpp OgreApp.cpp OgreAppFrameListener.cpp

OBJ = $(SRC:.cpp=.o)


# Library and includes
FLAGS = $(shell pkg-config opencv OGRE OIS ogg openal vorbisfile vorbis --cflags --libs) -Wall -O2

# Suffixes
.SUFFIXES: .cpp

anarcards: main.cpp libaruco_ogre.a SoundManager.o Card.o
	$(CC) $(OBJ) $^ -l:libaruco.so.1.2.4 -laruco_ogre -lOgreMain -lOIS $(FLAGS) -o $@

.cpp.o:
	$(CC) $(FLAGS) -c $< -o $@

libaruco_ogre.a: $(OBJ)
	ar rcs libaruco_ogre.a $(OBJ)

soundtest: SoundManager.o soundtest.cpp
	$(CC) $^ -o $@ $(FLAGS)

SoundManager.o: SoundManager.cpp
	$(CC) $(FLAGS) -c SoundManager.cpp

%.o: %.cpp
	$(CC) $(FLAGS) -c $^

clean:
	rm -f $(OBJ) aruco_test_ogre.o arucocards soundtest
