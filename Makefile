INC=-I./include/

all: 
	g++ main.cpp src/glad.c -Wall -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -I ./include/
