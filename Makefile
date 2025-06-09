INC=-I./include/

all: 
	g++ main.cpp src/glad.c -std=c++23 -Wall -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -I ./include/
