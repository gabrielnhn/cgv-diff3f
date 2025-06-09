#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 

// #include "npy.hpp"
#include "load_off_model.hpp"

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

auto camera = glm::vec3(0.0f, 0.0f, 3.0f);
auto aim = glm::vec3(0.0f);
auto nearDistance = 0.1f;
auto farDistance = 20.0f;
auto fov = glm::radians(60.0f);

double mousex, mousey;
double mousex_last, mousey_last;
int last_mouse_event = GLFW_RELEASE;

double height = 800;
double width = 800;

// // 346x260
// float ds_width = 346;
// float ds_height = 260;

float speed = 0.02f;

// static float yaw = 90.0f; // Start facing backward?
static float yaw = -90.0f; 
static float pitch = 0.0f;
glm::mat4 mvp;

float aspect_ratio = width/height;

void framebuffer_size_callback(GLFWwindow* window, int w, int h)
{
    width = w;
    height = h;
    glViewport(0, 0, width, height);
    // std::cout << "CHANGED WINDOW SIZE";
    std::cout << aspect_ratio << std::endl;
    aspect_ratio = width/height;
    std::cout << aspect_ratio << std::endl;

}  

// Move camera position according to keyboard and stuff
void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    glm::vec3 forward = glm::normalize(aim - camera);
    // glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0, 1, 0),forward)); // Right vector
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0))); // Right vector


    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        camera -= speed * forward;
        // aim -= speed * forward;
    }
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        camera += speed * forward;
        // aim += speed * forward;
    }
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        camera += speed * right;
        // aim += speed * right;
    }
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        camera -= speed * right;
        // aim -= speed * right;
    }
    aim = camera + forward;


    glfwGetCursorPos(window, &mousex, &mousey);
   
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
        last_mouse_event = 0;
   
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        if (last_mouse_event == 0)
        {
            mousex_last = mousex;
            mousey_last = mousey;
            last_mouse_event = 1;           
        }
        else
        {   
            float xdiff = (mousex - mousex_last)/width;
            float ydiff = (mousey - mousey_last)/height;
            
            float sensitivity = 50.0f; // Tune sensitivity
            yaw += xdiff * sensitivity;
            pitch -= ydiff * sensitivity; // Invert Y for natural movement

            pitch = glm::clamp(pitch, -89.0f, 89.0f); // Prevent flipping
            
            glm::vec3 direction;
            direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
            direction.y = sin(glm::radians(pitch));
            direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

            aim = camera + direction;
            
            mousex_last = mousex;
            mousey_last = mousey;
        }
        
    }
    // std::cout << "AIM: " << aim.x << ", " << aim.y << ", " << aim.z << ", (mousex =" << mousex << std::endl;
    // std::cout << "CAM: " << camera.x << ", " << camera.y << ", " << camera.z << ", (mousex =" << mousex << std::endl;

    //clamp
    camera.x = std::clamp(camera.x, -1.0f, 2.0f);
    aim.x = std::clamp(aim.x, -1.0f, 2.0f);

    camera.y = std::clamp(camera.y, -1.0f, 2.0f);
    aim.y = std::clamp(aim.y, -1.0f, 2.0f);
}

int main()
{
    GLenum err;

    const std::string path = "/home/gabrielnhn/cgv/SHREC_r/off_2/1.off";
    
    std::cout << "READING: " << path << std::endl;
    
    OffModel off_model(path);
    
    std::cout << "Finished" << std::endl;
    std::cout << "raw v0: "
            << off_model.vertices[0].x << ", "
            << off_model.vertices[0].y << ", "
            << off_model.vertices[0].z << '\n';

    glm::vec3 bbMin(  std::numeric_limits<float>::max());
    glm::vec3 bbMax( -std::numeric_limits<float>::max());

    for (auto &v : off_model.vertices) {
        bbMin = glm::min(bbMin, v);
        bbMax = glm::max(bbMax, v);
    }
    glm::vec3 centre = 0.5f * (bbMin + bbMax);
    float     diag   = glm::length(bbMax - bbMin);   // ≈ “size” of the model

    // --- build a model matrix that recentres & rescales -------------------------
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, -centre);          // move centre → origin
    model = glm::scale(model, glm::vec3(2.0f / diag)); // make it fit a 2×2×2 cube


    glm::vec4 obj = model * glm::vec4(off_model.vertices[0], 1.0f);
    std::cout << "fitted v0: "
          << obj.x << ", " << obj.y << ", " << obj.z << '\n';


    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
    GLFWwindow* window = glfwCreateWindow(width, height, "Diff3F Experiments", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window..." << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }   

    glViewport(0, 0, width, height);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    aspect_ratio = width/height;
    glm::mat4 projection = glm::perspective(fov, aspect_ratio, nearDistance, farDistance);

    glm::mat4 view = glm::lookAt(camera, aim, glm::vec3(0, 1, 0));
    // view = flipZ* view;

    // glm::mat4 model = glm::mat4(1.0f);
    mvp = projection * view * model;

    unsigned int VBO;
    glGenBuffers(1, &VBO);

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);  
    glBindVertexArray(VAO);

    unsigned int EBO;
    glGenBuffers(1, &EBO);

    // VBO FOR VERTICES
    glBindBuffer(GL_ARRAY_BUFFER, VBO);  
    glBufferData(GL_ARRAY_BUFFER, off_model.vertices.size()*sizeof(glm::vec3), &off_model.vertices[0], GL_STATIC_DRAW);

    // glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float)*4, (void*)0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, (void*)0);
    glEnableVertexAttribArray(0);  

    // EBO FOR FACES
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, off_model.faces.size()*sizeof(glm::ivec3), &off_model.faces[0], GL_STATIC_DRAW); 


    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error before shader definition" << err << std::endl;
    }
    
    const char *vertexShaderSourceGLSLCode =
        "#version 330 core\n"
        "layout (location = 0) in vec3 vertexPosition; // Expecting vec4 for each vertex\n"
        "uniform mat4 mvp;  // Model-View-Projection matrix\n"
        "out float vertexShade;  // Output color to fragment shader\n"
        "void main()\n"
        "{\n"
        "    vertexShade = vertexPosition.z;\n"  // Pass the position directly to the fragment shader for color"
        // "    gl_PointSize = 5.0;\n"
        "    gl_Position = mvp * vec4(vertexPosition, 1.0);\n"  // Apply MVP transformation"
        "}\0";

    
    const char *fragShaderSourceGLSLCode = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in float vertexShade;\n"
        "void main()\n"
        "{\n"
            "FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}\0";
    

    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSourceGLSLCode, NULL);
    glCompileShader(vertexShader);
    //
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    //
    unsigned int fragShader;
    fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fragShaderSourceGLSLCode, NULL);
    glCompileShader(fragShader);
    //
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAG::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    //
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragShader);

    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FULLSHADERPROGRAM::LINK_FAILED\n" << infoLog << std::endl;
    }
    glUseProgram(shaderProgram);
    glEnable(GL_PROGRAM_POINT_SIZE);



    int mvpLocation = glGetUniformLocation(shaderProgram, "mvp");
    glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));

    // glUniform1i(glGetUniformLocation(shaderProgram, "total_vertices"), off_model.vertices.size());
    // glUniform1i(glGetUniformLocation(shaderProgram, "column_size"), off_model.vertices.size()/3);
    // glUniform1i(glGetUniformLocation(shaderProgram, "column_size"), off_model.vertices.size());

    glEnable(GL_DEPTH_TEST);
    // glDisable(GL_CULL_FACE);
    // glFrontFace(GL_CCW); // Changes default to Clockwise

    // glClearColor(0.2f, 0.2f, 0.2f, 0.5f);
    glClearColor(1.0f,1.0f,1.0f,1.0f);
    // int once = 0;
    while(!glfwWindowShouldClose(window))
    {
        // if (not once)
        // {
        //     glm::vec4 clip = projection * view * obj;
        //     glm::vec3 ndc  = glm::vec3(clip) / clip.w;
        //     std::cout << "NDC v0: " << ndc.x << ", " << ndc.y << ", " << ndc.z << '\n';
        //     once +=1;
        // }

        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cerr << "OpenGL error: " << err << std::endl;
        }

        // remake projection
        glm::mat4 projection = glm::perspective(fov, aspect_ratio, nearDistance, farDistance);
        view = glm::lookAt(camera, aim, glm::vec3(0, 1, 0));
        mvp = projection * view * model;
        int mvpLocation = glGetUniformLocation(shaderProgram, "mvp");
        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));


        processInput(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        // FOR POINT CLOUD
        // glPointSize(10.0f); // Set point size to 10 pixels
        // glBindVertexArray(VAO);
        // glDrawArrays(GL_POINTS, 0, off_model.vertices.size());

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glDrawElements(GL_TRIANGLES, off_model.faces.size()*3, GL_UNSIGNED_INT, 0);



        glfwSwapBuffers(window);
        glfwPollEvents();    
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragShader); 

    return 0;
}