# Diffusion 3D Features, Visualized

Requirements (fill instructions later)
Download dataset in (path)

all libs in makefile!
OpenGL (3?)

## Installation (apt-based Linux setup)

Install g++ with C++23 support; (Comes by default in modern linux distros)
Install required packages GLFW, GLM, STB;

```bash
sudo apt update
sudo apt install libglfw3 libglfw3-dev libglm-dev libstb-dev libfreetype6-dev
```

Install conda/miniconda;
Create and activate the environment.
```bash
conda env create -f environment.yml
conda activate diff3df
```

Compile the program;

```bash
make
```

Run the program:

```bash
export LD_LIBRARY_PATH=$CONDA_PREFIX/lib:$LD_LIBRARY_PATH
./DIFF3DF_VISUALIZED
```