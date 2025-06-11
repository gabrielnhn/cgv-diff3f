// https://learnopengl.com/In-Practice/Text-Rendering

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 

#include <map> 
// #include "load_shader.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H 

FT_Library ft;
FT_Face face;

struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2   Size;       // Size of glyph
    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    long int Advance;    // Offset to advance to next glyph
};
std::map<char, Character> Characters;


std::vector<unsigned int> textVertexShaders;
std::vector<unsigned int> textFragShaders;
std::vector<unsigned int> textPrograms;
std::vector<unsigned int> textVAOs;
std::vector<unsigned int> textVBOs;


int textSetup(int windowIndex)
{    
    int i = windowIndex;
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
    std::cerr << "OpenGL error before textSetup: " << err << std::endl;
    return 1;
}
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }
    
    if (FT_New_Face(ft, "external/arial.ttf", 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;  
        return -1;
    }
    
    FT_Set_Pixel_Sizes(face, 0, 48);
    if (FT_Load_Char(face, 'X', FT_LOAD_RENDER))
    {
        std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;  
        return -1;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction
  
    for (unsigned char c = 0; c < 128; c++)
    {
        // load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // now store character for later use
        Character character = {
            texture, 
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };
        Characters.insert(std::pair<char, Character>(c, character));
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    }


    // set up shader
    int success;
    char infoLog[512];

    auto textVertexSource = ShaderSourceCode("./external/shaders/text_vertex.glsl");
    auto textFragSource = ShaderSourceCode("./external/shaders/text_frag.glsl");

    textVertexShaders[i] = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(textVertexShaders[i], 1, &textVertexSource.text, NULL);
    glCompileShader(textVertexShaders[i]);
    glGetShaderiv(textVertexShaders[i], GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(textVertexShaders[i], 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        return 0;
    }

    textFragShaders[i] = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(textFragShaders[i], 1, &textFragSource.text, NULL);
    glCompileShader(textFragShaders[i]);
    glGetShaderiv(textFragShaders[i], GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(textFragShaders[i], 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        return 0;
    }
    //
    textPrograms[i] = glCreateProgram();
    glAttachShader(textPrograms[i], textVertexShaders[i]);
    glAttachShader(textPrograms[i], textFragShaders[i]);
    glLinkProgram(textPrograms[i]);

    glGetProgramiv(textPrograms[i], GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(textPrograms[i], 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FULLTEXTPROGRAM::LINK_FAILED\n" << infoLog << std::endl;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // TODO replace with width and height later
    double height = 800;
    double width = 800;
    // Update text projection matrix
    glUseProgram(textPrograms[i]); // Activate text shader to set its uniform
    glm::mat4 textProjection = glm::ortho(0.0f, (float)width, 0.0f, (float)height);
    int projectionLocation = glGetUniformLocation(textPrograms[i], "projection");
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(textProjection));
    glUseProgram(0); // Deactivate text shader
    // glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 800.0f);
    // int projectionLocation = glGetUniformLocation(textProgram, "projection");
    // glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

    glGenVertexArrays(1, &textVAOs[i]);
    glGenBuffers(1, &textVBOs[i]);
    glBindVertexArray(textVAOs[i]);
    glBindBuffer(GL_ARRAY_BUFFER, textVBOs[i]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error still in textSetup: " << err << std::endl;
        return 1;
    }

    return 1;
}

void textFinish()
{
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}
    
int RenderText(int windowIndex, std::string text, float x, float y, float scale, glm::vec3 color)
{
    int i = windowIndex;
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error before RenderText: " << err << std::endl;
        return 0;
    }
    // activate corresponding render state	
    glUseProgram(textPrograms[i]);

    glUniform3f(glGetUniformLocation(textPrograms[i], "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVAOs[i]);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }           
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, textVBOs[i]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error after RenderText: " << err << std::endl;
        return 0;
    }
    return 1;
}