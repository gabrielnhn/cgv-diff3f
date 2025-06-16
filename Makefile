# MYINC= \
# 	-I./external/include/ \
# 	-I./include/ \
# 	-I/usr/include/freetype2 \
# 	-I$(CONDA_PREFIX)/include/python3.12 -L$(CONDA_PREFIX)/lib/
	
# # -I/usr/include/python3.10 

# OPENCVPARAMS = $(shell pkg-config --cflags --libs opencv4)
# MYLIBS = -fopenmp -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lstb -lfreetype \
# 	-lpython3.12
# # -lpython3.10


MYINC = \
    -I./external/include/ \
    -I./include/ \
    -I/usr/include/freetype2 \
    -I$(CONDA_PREFIX)/include/python3.12 \
    -I$(CONDA_PREFIX)/include/opencv4

MYLIBS = \
    -L$(CONDA_PREFIX)/lib \
    -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs \
    -lpython3.12 \
    -fopenmp -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lstb -lfreetype
	
MYPARAMS= -std=c++23 -g -Wall -Wextra

LDFLAGS += -Wl,-rpath,$(CONDA_PREFIX)/lib

all: 
	g++ src/main.cpp external/src/glad.c $(MYINC) $(MYPARAMS) $(MYLIBS)  $(LDFLAGS)


