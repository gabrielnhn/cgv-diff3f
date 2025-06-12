# # Dynamically find Python executable from conda environment "control"
# PYTHON = $(shell conda run -n control which python)
# PYTHON_CONFIG = $(shell conda run -n control python3-config --prefix)

# # Include and lib directories from conda environment
# PYTHON_INCLUDE = -I$(PYTHON_CONFIG)/include/python$(shell conda run -n control python -c "import sys;print(f'{sys.version_info[0]}.{sys.version_info[1]}')")
# PYTHON_LIB = -L$(PYTHON_CONFIG)/lib -lpython$(shell conda run -n control python -c "import sys;print(f'{sys.version_info[0]}.{sys.version_info[1]}')")

# # Extra includes
# MYINC = -I./external/include/ -I./include/ -I/usr/include/freetype2 $(PYTHON_INCLUDE)

# # Compilation params and libs
# MYPARAMS = -std=c++23 -Wall
# MYLIBS = -fopenmp -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lstb -lfreetype $(PYTHON_LIB)

# all:
# 	echo "using $(PYTHON_LIB) $(PYTHON_INCLUDE)"
# 	g++ src/main.cpp external/src/glad.c $(MYINC) $(MYPARAMS) $(MYLIBS)

# MYINC=-I./external/include/ -I./include/ -I/usr/include/freetype2 -I./external/Axodox.MachineLearning.2.0.55/include/Include -I./external/Axodox.MachineLearning.2.0.55/include/MachineLearning -I/usr/include/python3.10
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


