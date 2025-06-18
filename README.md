# Diffusion 3D Features, Visualized

## Installation (apt-based Linux setup)

Install g++ with C++23 support; (Comes by default in modern linux distros)
Install required packages GLFW, GLM, STB;

```bash
sudo apt update
sudo apt install libglfw3 libglfw3-dev libglm-dev libstb-dev libfreetype6-dev
```

If GLAD fails (probably won't), download if from https://glad.dav1d.de/ using version 3.3 core, and replace `glad.c` and `glad.h` in `external/`


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

### Usage
You can use the mouse and left click to rotate the model around and get different perspectives.

Pressing the numbers 1,2 or 3 will allow you to change the method to compute features, but DINO (the defaul=1) is the only one where features have any semantic meaning; the others are only for the sake of visualizing how the features area aggregated at each vertex according to multiple projections/feature computations.

You can change the 3D object instance from the dataset using the left and right keys on the keyboard.

By pressing right click on the mouse, the python script will run, compute the features, and those will be projected back to the 3D shape vertices. After computing the features (for both models), you can select a point in one of the windows and set it as reference to compute point-to-point correspondence for the other object in the other window.