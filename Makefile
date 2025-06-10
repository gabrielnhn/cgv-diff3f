MYINC=-I./external/include/ -I./include/ -I/usr/include/freetype2 -I/usr/include/opencv4
MYPARAMS= -std=c++23 -Wall 
MYLIBS = -fopenmp -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lstb -lfreetype -lopencv_core

all: 
	g++ src/main.cpp external/src/glad.c $(MYINC) $(MYPARAMS) $(MYLIBS)
