# Experiments on Diff3f

Requirements (fill instructions later)
Download dataset in (path)

all libs in makefile!
OpenGL (3?)
GLAD framework (supposedly on external/)
GLM
g++, C++23
OpenMP
libstb
libfreetype6-dev
libpython3.10

sudo apt-get install xtensor-dev

# https://huggingface.co/lllyasviel/ControlNet/resolve/main/models/control_sd15_depth.pth?download=true
conda env create -f environment.yaml
conda activate control

#sudo apt install mono-devel
https://learn.microsoft.com/en-us/nuget/install-nuget-client-tools?tabs=macos#cli-tools
cd external
nuget install Axodox.MachineLearning

libopencv-dev

conda env create -f environment.yml
conda activate diff3df
export LD_LIBRARY_PATH=$CONDA_PREFIX/lib:$LD_LIBRARY_PATH