// https://lencerf.github.io/post/2019-09-21-save-the-opengl-rendering-to-image-file/

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

void saveImage(std::string path, GLFWwindow* w, bool flip=true) {
 const char* filepath = path.c_str();
 int width, height;
 glfwGetFramebufferSize(w, &width, &height);
 GLsizei nrChannels = 3;
 GLsizei stride = nrChannels * width;
 stride += (stride % 4) ? (4 - stride % 4) : 0;
 GLsizei bufferSize = stride * height;
 std::vector<char> buffer(bufferSize);
 glPixelStorei(GL_PACK_ALIGNMENT, 4);
 glReadBuffer(GL_FRONT);
 glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
 stbi_flip_vertically_on_write(flip);
 stbi_write_png(filepath, width, height, nrChannels, buffer.data(), stride);
}

std::vector<unsigned char> loadImageData(const std::string& path,
    int& out_width, int& out_height, int& out_channels)
{
    unsigned char* data = stbi_load(path.c_str(), &out_width, &out_height, &out_channels, 0);

    if (!data) {
        std::cerr << "Error loading image: " << path << std::endl;
        return {};
    }

    std::vector<unsigned char> image_buffer(data, data + (out_width * out_height * out_channels));
    stbi_image_free(data); // Free the memory allocated by stbi_load

    return image_buffer;
}