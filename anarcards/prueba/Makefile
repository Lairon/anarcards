# Compiler
CC = g++

# Source
SRC = OgreARAppLogic.cpp OgreApp.cpp OgreAppFrameListener.cpp

OBJ = $(SRC:.cpp=.o)

# Include paths
INCLUDES = -I. -I/usr/include/ -I/usr/local/include/OGRE/ -I/usr/local/include/

# Library paths
LIBS = -L. -L/usr/lib -L/usr/local/lib -lm -I/usr/local/include/opencv -I/usr/local/include  /usr/local/lib/libopencv_calib3d.so /usr/local/lib/libopencv_contrib.so /usr/local/lib/libopencv_core.so /usr/local/lib/libopencv_features2d.so /usr/local/lib/libopencv_flann.so /usr/local/lib/libopencv_gpu.so /usr/local/lib/libopencv_highgui.so /usr/local/lib/libopencv_imgproc.so /usr/local/lib/libopencv_legacy.so /usr/local/lib/libopencv_ml.so /usr/local/lib/libopencv_nonfree.so /usr/local/lib/libopencv_objdetect.so /usr/local/lib/libopencv_photo.so /usr/local/lib/libopencv_stitching.so /usr/local/lib/libopencv_ts.so /usr/local/lib/libopencv_video.so /usr/local/lib/libopencv_videostab.so

# Suffixes
.SUFFIXES: .cpp

arucocards: aruco_test_ogre.o libaruco_ogre.a
	$(CC) $(OBJ) aruco_test_ogre.o -l:libaruco.so.1.2.4 -laruco_ogre -lOgreMain -lOIS $(LIBS) -o arucocards

aruco_test_ogre.o: aruco_test_ogre.cpp
	$(CC) $(INCLUDES) -c aruco_test_ogre.cpp

.cpp.o:
	$(CC) $(INCLUDES) -c $< -o $@

libaruco_ogre.a: $(OBJ)
	ar rcs libaruco_ogre.a $(OBJ)

clean:
	rm $(OBJ) aruco_test_ogre.o arucocards
