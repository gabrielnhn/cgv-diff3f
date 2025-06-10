MYINC=-I./external/include/ -I./include/
MYPARAMS= -std=c++23 -Wall 
MYLIBS = -fopenmp -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl

all: 
	g++ main.cpp external/src/glad.c $(MYINC) $(MYPARAMS) $(MYLIBS)
