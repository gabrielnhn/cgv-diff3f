
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 
// #include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#include <stb/stb_image.h>
// #include "magma.hpp"
#include "colormaps.hpp"
// #include <algorithm>

class myImage
{
    public:
    
    int width, height, channels;
    std::vector<unsigned char> data;
    float toFloat = 1.0f / 255.0f;
    float fromFloat = 255.0f;
    float minVal, maxVal;


    myImage(std::string path)
    {
        // std::cout << "LOADING FROM IMAGE in PATH " << path << std::endl;
        unsigned char* imdata = stbi_load(path.c_str(), &width, &height, &channels, 0);

        if (not imdata) {
            std::cerr << "Error loading image: " << path << std::endl;
            return;
        }

        std::vector<unsigned char> image_buffer(imdata, imdata + (width * height * channels));
        // data = image_buffer;
        data = std::move(image_buffer);
        stbi_image_free(imdata);

        maxVal = data[0];
        #pragma omp parallel for reduction(max: maxVal)
        for(long unsigned int i = 1; i < data.size(); i++)
        {
            float value = data[i];   
            maxVal = std::max(maxVal, value);
        }
        minVal = maxVal;
        #pragma omp parallel for reduction(min: minVal)
        for(long unsigned int i = 0; i < data.size(); i++)
        {
            float value = data[i];   
            if(value > 10)
                minVal = std::min(minVal, value);
        }
        // std::cout << "minVal is " << minVal << " and maxVal " << maxVal << std::endl;
    }

    glm::vec3 getValue(int i, int j)
    {
        long unsigned int index = (long unsigned int)(i * width + j) * channels;

        float r = data[index] * toFloat;
        float g = data[index + 1] * toFloat;
        float b = data[index + 2] * toFloat;

        return glm::vec3(r,g,b);
    }

    glm::vec3 toMagma(int i, int j)
    {
        long unsigned int index = (long unsigned int)(i * width + j) * channels;
        int unscaled = data[index];

        if (maxVal == minVal)
            return magma[unscaled];

        float scaled = ((unscaled - minVal) / (maxVal - minVal));
        // std::cout << "unscaled " << unscaled << std::endl;
        // std::cout << "scaled " << scaled << std::endl;

        index = scaled * fromFloat;

        index = std::clamp((int)index, 0, 255);
        // reverse (closer == smaller value)
        return magma[255-index];
    }
};


