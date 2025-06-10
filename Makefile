MYINC=-I./external/include/ -I./include/
MYPARAMS= -std=c++23 -Wall 
MYLIBS = -fopenmp -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lstb

all: 
	g++ src/main.cpp external/src/glad.c $(MYINC) $(MYPARAMS) $(MYLIBS)
