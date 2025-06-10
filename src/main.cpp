#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include <cassert>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 

#include "load_off_model.hpp"
#include "load_shader.hpp"
#include "save_image.hpp"
#include "load_image.hpp"
#include "draw_text.hpp"

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

float max_distance_to_object = 2;
auto default_camera = glm::vec3(0.0f, 0.0f, max_distance_to_object);
auto camera = default_camera;
auto aim = glm::vec3(0.0f);
auto nearDistance = 0.1f;
// auto farDistance = 20.0f;
auto farDistance = 5.0f;
auto fov = glm::radians(60.0f);

double mousex, mousey;
double mousex_last, mousey_last;
int last_mouse_event = GLFW_RELEASE;
float sensitivity = 10.0f;

double height = 800;
double width = 800;

float speed = 0.05f;

glm::mat4 mvp;
glm::mat4 mv;

float aspect_ratio = width/height;

// image saving stuff
bool should_save_next_frame = false;



void framebuffer_size_callback(GLFWwindow* window, int w, int h)
{
    width = w;
    height = h;
    glViewport(0, 0, width, height);
    std::cout << "CHANGED WINDOW SIZE to w,h" << w << ", " << h << std::endl;
    std::cout << aspect_ratio << std::endl;
    aspect_ratio = width/height;
    std::cout << aspect_ratio << std::endl;

    // Update text projection matrix
    glUseProgram(textProgram); // Activate text shader to set its uniform
    glm::mat4 textProjection = glm::ortho(0.0f, (float)width, 0.0f, (float)height);
    int projectionLocation = glGetUniformLocation(textProgram, "projection");
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(textProjection));
    glUseProgram(0); // Deactivate text shader

}  

// move camera position according to keyboard and stuff
void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    glm::vec3 forward = glm::normalize(aim - camera);
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
    glm::vec3 up = glm::normalize(glm::cross(forward, glm::vec3(1, 0, 0)));
    // up = glm::abs(up);

    // auto forward = glm::vec3(0,0,1);
    // auto right = glm::vec3(1,0,0);
    // auto up = glm::vec3(0,1,0);


    // keyboard
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera -= speed * up;
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera += speed * up;

    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera += speed * right;
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera -= speed * right;

    if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera += speed * forward;
    if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera -= speed * forward;

    if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        camera = default_camera;
    
    if(glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
    {
        should_save_next_frame = true;
    }

    //mouse
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

            camera -= xdiff * right * sensitivity;
            camera -= ydiff * up * sensitivity;
            
            mousex_last = mousex;
            mousey_last = mousey;
        }
    }
    
    // clamp with radius=max_distance
    if (glm::length(camera) > max_distance_to_object)
        camera = glm::normalize(camera) * max_distance_to_object;

}

int unproject_image(glm::mat4 current_projection, glm::mat4 current_mv,
    std::string feature_image_path, std::string depth_image_path,
    GLFWwindow* window, OffModel* off_object)
{
    myImage depth_image(depth_image_path);

    glfwGetCursorPos(window, &mousex, &mousey);
    // auto x_feat = width/2;
    // auto y_feat = height/2;
    float x_feat = mousex;
    float y_feat = mousey;

    float image_value = depth_image.getValue(int(y_feat), int(x_feat)).r;
    // image_value = (0.70 - (-depth/farPlaneDistance);
    // image_value = (0.70 + depth/farPlaneDistance;
    // depth/farPlaneDistance = image_value - 0.70;
    float depth = (image_value - 0.70) * farDistance;


    glm::vec3 unproj_coord = glm::vec3(x_feat, y_feat, depth);
    glm::vec4 viewport_rect = glm::vec4(0.0f, 0.0f, width, height);

    glm::vec3 world_point = glm::unProject(
        unproj_coord,
        current_mv,
        current_projection,
        viewport_rect
    );

    std::cout << "world point is: " << world_point.x << ", " << world_point.y << ", " << world_point.z << std::endl; 
 
    int closest_point_index = 0;
    auto closest_distance = glm::distance(world_point, off_object->vertices[closest_point_index]);
    for (long unsigned int i = 1; i < off_object->vertices.size(); i++)
    {
        float this_distance = glm::distance(world_point, off_object->vertices[i]);
        if (this_distance < closest_distance)
        {
            closest_point_index = i;
        }
    }
    // auto closest_point = off_object->vertices[closest_point_index];
    off_object->features[closest_point_index] = glm::vec3(1.0, 0.0, 0.0);

    return 1;
}

int main()
{
    GLenum err;

    // PREPARE WINDOW
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

    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN); 
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); 
    aspect_ratio = width/height;
    
    // PREPARE SHADERS

    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error before shader definition" << err << std::endl;
    }
    
    // VERTEX OPTIONS
    auto vertexShaderSource = ShaderSourceCode("shaders/vertex_shader_depth.glsl");
    // auto vertexShaderSource = ShaderSourceCode("shaders/vertex_shader_mvp.glsl");
    
    // GEOM OPTIONS
    auto geometryShaderSource = ShaderSourceCode("shaders/geometry_shader_normal.glsl");
    
    // FRAG OPTIONS
    auto fragShaderSource = ShaderSourceCode("shaders/frag_shader_depth.glsl");
    // auto fragShaderSource = ShaderSourceCode("shaders/frag_shader_phong.glsl");

    int success;
    char infoLog[512];

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource.text, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    //
    unsigned int geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geometryShader, 1, &geometryShaderSource.text, NULL);
    glCompileShader(geometryShader);
    glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(geometryShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    //
    unsigned int fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fragShaderSource.text, NULL);
    glCompileShader(fragShader);
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAG::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    //
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    // glAttachShader(shaderProgram, geometryShader);
    glAttachShader(shaderProgram, fragShader);

    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FULLSHADERPROGRAM::LINK_FAILED\n" << infoLog << std::endl;
    }
    glUseProgram(shaderProgram);
    glEnable(GL_PROGRAM_POINT_SIZE);

    // load model data

    // const std::string path = "/home/gabrielnhn/cgv/SHREC_r/off_2/1.off";
    // const std::string path = "/home/gabrielnhn/cgv/SHREC_r/off_2/2.off";
    // const std::string path = "/home/gabrielnhn/cgv/SHREC_r/off_2/3.off";
    const std::string path = "/home/gabrielnhn/cgv/SHREC_r/off_2/4.off";
    
    std::cout << "READING: " << path << std::endl;
    
    OffModel off_object(path);

    // find model bounding box
    glm::vec3 bbMin( std::numeric_limits<float>::max());
    glm::vec3 bbMax(-std::numeric_limits<float>::max());

    for (auto &v: off_object.vertices) {
        bbMin = glm::min(bbMin, v);
        bbMax = glm::max(bbMax, v);
    }
    
    glm::vec3 center = (bbMin + bbMax) * 0.5f;
    float diag = glm::length(bbMax - bbMin);

    // move and scale to fit bbox
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, -center); 
    model = glm::scale(model, glm::vec3(2.0f / diag));

    // prepare mvp matrices
    glm::mat4 projection = glm::perspective(fov, aspect_ratio, nearDistance, farDistance);
    glm::mat4 view = glm::lookAt(camera, aim, glm::vec3(0, 1, 0));
    mvp = projection * view * model;
    mv = view * model;

    // set persistent shader uniforms
    auto ambient_light = glm::vec3(0.3f, 0.3f, 0.3f);

    int farLocation = glGetUniformLocation(shaderProgram, "farPlaneDistance");
    glUniform1f(farLocation, farDistance);
    glUniform3f(glGetUniformLocation(shaderProgram, "object_color"), 1.0f, 0.5f, 0.5f); 
    // glUniform3f(glGetUniformLocation(shaderProgram, "ambient_light"), 0.2f, 0.2f, 0.2f); 
    glUniform3f(glGetUniformLocation(shaderProgram, "ambient_light"), ambient_light.x,ambient_light.y,ambient_light.z); 

    // buffer model data to gpu
    unsigned int VBO;
    glGenBuffers(1, &VBO);

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);  
    glBindVertexArray(VAO);

    unsigned int EBO;
    glGenBuffers(1, &EBO);

    // VBO FOR VERTICES
    glBindBuffer(GL_ARRAY_BUFFER, VBO);  
    glBufferData(GL_ARRAY_BUFFER, off_object.vertices.size()*sizeof(glm::vec3), &off_object.vertices[0], GL_STATIC_DRAW);

    // glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float)*4, (void*)0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, (void*)0);
    glEnableVertexAttribArray(0);  

    // EBO FOR FACES
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, off_object.faces.size()*sizeof(glm::ivec3), &off_object.faces[0], GL_STATIC_DRAW); 

    glEnable(GL_DEPTH_TEST);

    // glClearColor(1.0f,1.0f,1.0f,1.0f);
    // glClearColor(ambient_light.x, ambient_light.y, ambient_light.z,1.0f);
    glClearColor(0.0f,0.0f,0.0f,0.0f);

    int text_loaded = textSetup();
    assert(text_loaded);

    int loop_count = 0;
    while(!glfwWindowShouldClose(window))
    {

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        processInput(window); // recompute camera position

        glUseProgram(shaderProgram);
        // remake projection
        projection = glm::perspective(fov, aspect_ratio, nearDistance, farDistance);
        view = glm::lookAt(camera, aim, glm::vec3(0, 1, 0));
        mvp = projection * view * model;
        mv = view * model;

        // set variable shader uniforms
        int mvpLocation = glGetUniformLocation(shaderProgram, "mvp");
        int mvLocation = glGetUniformLocation(shaderProgram, "mv");
        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));
        glUniformMatrix4fv(mvLocation, 1, GL_FALSE, glm::value_ptr(mv));
        glUniform3f(glGetUniformLocation(shaderProgram, "light_pos"), camera.x, camera.y, camera.z);
        
        glBindVertexArray(VAO);

        // FOR POINT CLOUD
        // glPointSize(10.0f);
        // glDrawArrays(GL_POINTS, 0, off_object.vertices.size());

        // FOR SURFACES
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glDrawElements(GL_TRIANGLES, off_object.faces.size()*3, GL_UNSIGNED_INT, 0);

        
        // render text
        if (not should_save_next_frame)
        {
            int text_rendered = RenderText("Press [Enter] to generate texture", 25.0f, 25.0f, 0.5f, glm::vec3(0.5, 0.8f, 0.2f));
            assert(text_rendered);
        }
        

        glfwSwapBuffers(window);
        glfwPollEvents();

        if (should_save_next_frame)
        {
            saveImage("./bruh.png", window, false);
            should_save_next_frame = false;
            unproject_image(projection, mv, "", "./bruh.png",
                window, &off_object);
        }



        loop_count += 1;
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cerr << "OpenGL error: " << err << "at loop count " << loop_count << std::endl;
            return 1;
        }

       
    }

    glDeleteShader(vertexShader);
    glDeleteShader(geometryShader); 
    glDeleteShader(fragShader); 
    glDeleteProgram(shaderProgram);
    textFinish();

    return 0;
}