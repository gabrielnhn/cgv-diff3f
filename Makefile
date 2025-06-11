MYINC=-I./external/include/ -I./include/ -I/usr/include/freetype2 -I/usr/include/python3.10
MYPARAMS= -std=c++23 -Wall 
MYLIBS = -fopenmp -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lstb -lfreetype -lpython3.10

all: 
	g++ src/main.cpp external/src/glad.c $(MYINC) $(MYPARAMS) $(MYLIBS)
