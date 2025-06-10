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

void saveImage(std::string path, GLFWwindow* w) {
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
 stbi_flip_vertically_on_write(true);
 stbi_write_png(filepath, width, height, nrChannels, buffer.data(), stride);
}