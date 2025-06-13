MYINC= \
	-I./external/include/ \
	-I./include/ \
	-I/usr/include/freetype2 \
	-I/usr/include/python3.10 \

OPENCVPARAMS = $(shell pkg-config --cflags --libs opencv4)

# MYPARAMS= -std=c++23 -Wall -Wextra -g
MYPARAMS= -std=c++23 -g
MYLIBS = -fopenmp -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lstb -lfreetype \
	-lpython3.10

all: 
	g++ src/main.cpp external/src/glad.c $(OPENCVPARAMS) $(MYINC) $(MYPARAMS) $(MYLIBS) 


