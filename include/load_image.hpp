
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

class myImage
{
    public:
    
    int width, height, channels;
    std::vector<unsigned char> data;


    myImage(std::string path)
    {
        unsigned char* imdata = stbi_load(path.c_str(), &width, &height, &channels, 0);

        if (not imdata) {
            std::cerr << "Error loading image: " << path << std::endl;
            return;
        }

        std::vector<unsigned char> image_buffer(imdata, imdata + (width * height * channels));
        data = image_buffer;
        stbi_image_free(imdata); // Free the memory allocated by stbi_load

    }

    glm::vec3 getValue(int i, int j)
    {
        float toFloat = 1/255;

        int index = data[(i * width + j)*channels];
        
        float r = data[index] * toFloat;
        float g = data[index + 1] * toFloat;
        float b = data[index + 2] * toFloat;

        return glm::vec3(r,g,b);
    }
};


