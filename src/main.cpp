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
#include "run_python.hpp"
// #include "magma.hpp"

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

// float max_distance_to_object = 1.5;
float max_distance_to_object = 2.0;
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
bool should_reset = false;

unsigned int DepthShaderProgram;
unsigned int PHONGShaderProgram;
unsigned int currentRenderProgram = 0;

unsigned int VBOPos, VBOColor;


void framebuffer_size_callback(GLFWwindow* window, int w, int h)
{
    (void)window; // avoid warning
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

int updateModelVBO(OffModel* off_object)
{
    glBindBuffer(GL_ARRAY_BUFFER, VBOColor);  
    glBufferData(GL_ARRAY_BUFFER, off_object->features.size()*sizeof(glm::vec3), &off_object->features[0], GL_DYNAMIC_DRAW);
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, (void*)0);
    // glEnableVertexAttribArray(1); 
    // PREPARE SHADERS
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error in updateModelVBO()" << err << std::endl;
        return 0;
    }
    return 1;
}


void reset_features(OffModel* off_object)
{
    #pragma omp parallel for
    for(long unsigned int i = 0; i < off_object->features.size(); i++)
    {
        off_object->features[i] = off_object->default_feature;
        off_object->hits[i] = 0;
    }
    updateModelVBO(off_object);
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

    // if(glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE)
    if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        camera = default_camera;
        should_reset = true;
    }
    
    if((glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
        or (glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS))
    {
        should_save_next_frame = true;
        currentRenderProgram = DepthShaderProgram;
    }


    if(glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        currentRenderProgram = PHONGShaderProgram;
    }
    if(glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        currentRenderProgram = DepthShaderProgram;
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
    // std::string feature_image_path,
    std::string depth_image_path,
    // GLFWwindow* window,
    OffModel* off_object, float diag)
{
    // float random_float1 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    // float random_float2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    // float random_float3 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

    myImage depth_image(depth_image_path);

    // glfwGetCursorPos(window, &mousex, &mousey);
    // float x_feat = mousex;
    // float y_feat = mousey;
    glm::vec4 viewport(0, 0, width, height);
    
    #pragma omp parallel for
    for (unsigned long int i = 0; i < off_object->vertices.size(); ++i)
    {
        float small_noise = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        small_noise = small_noise / 10.0;

        glm::vec3 ndc = glm::project(off_object->vertices[i],
                                    current_mv,
                                    current_projection,
                                    viewport);
      

        if (ndc.x < 0 || ndc.x >= depth_image.width || ndc.y < 0 || ndc.y >= depth_image.height) continue;

        float depthBuf = depth_image.getValue(height - ndc.y,ndc.x).r;
        glm::vec3 magma = depth_image.toMagma(height - ndc.y, ndc.x);
        float projDepth = ndc.z;

        // if (projDepth < depthBuf + 0.01)
        // if (projDepth < depthBuf + 0.003)
        if (projDepth < depthBuf + diag*0.002)
        {
            off_object->hits[i] += 1;
            float weight = 1.0f / off_object->hits[i];
            auto prev_value = off_object->features[i] * (1.0f - weight);
            // auto new_value = glm::vec3(random_float1+small_noise, random_float2+small_noise, random_float3+small_noise) * weight;
            auto new_value = magma * weight;
            
            
            off_object->features[i] = prev_value + new_value;
        }
    }
    
    updateModelVBO(off_object);

    return 1;
}

int main(int argc, char* argv[])
{
    run_python(argc, argv, "./src/diffusion.py");

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
    
    int success;
    char infoLog[512];
    // PREPARE DEPTH SHADER

    // VERTEX OPTIONS
    auto DepthVertexShaderSource = ShaderSourceCode("shaders/vertex_shader_depth.glsl");
    // FRAG OPTIONS
    auto DepthFragShaderSource = ShaderSourceCode("shaders/frag_shader_depth.glsl");

    unsigned int DepthVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(DepthVertexShader, 1, &DepthVertexShaderSource.text, NULL);
    glCompileShader(DepthVertexShader);
    glGetShaderiv(DepthVertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(DepthVertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    unsigned int DepthFragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(DepthFragShader, 1, &DepthFragShaderSource.text, NULL);
    glCompileShader(DepthFragShader);
    glGetShaderiv(DepthFragShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(DepthFragShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAG::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    //
    DepthShaderProgram = glCreateProgram();
    glAttachShader(DepthShaderProgram, DepthVertexShader);
    glAttachShader(DepthShaderProgram, DepthFragShader);

    glLinkProgram(DepthShaderProgram);
    glGetProgramiv(DepthShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(DepthShaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FULLSHADERPROGRAM::LINK_FAILED\n" << infoLog << std::endl;
    }
    glUseProgram(DepthShaderProgram);
    glEnable(GL_PROGRAM_POINT_SIZE);

    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error after depth shader: " << std::endl;
        return 1;
    }


    // PREPARE PHONG SHADER
    auto PHONGVertexShaderSource = ShaderSourceCode("shaders/vertex_shader_mvp.glsl");
    auto PHONGGeometryShaderSource = ShaderSourceCode("shaders/geometry_shader_normal.glsl");
    // auto PHONGFragShaderSource = ShaderSourceCode("shaders/frag_shader_phong.glsl");
    auto PHONGFragShaderSource = ShaderSourceCode("shaders/frag_shader_features.glsl");
   
    unsigned int PHONGVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(PHONGVertexShader, 1, &PHONGVertexShaderSource.text, NULL);
    glCompileShader(PHONGVertexShader);
    glGetShaderiv(PHONGVertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(PHONGVertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    //
    unsigned int PHONGGeometryShader = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(PHONGGeometryShader, 1, &PHONGGeometryShaderSource.text, NULL);
    glCompileShader(PHONGGeometryShader);
    glGetShaderiv(PHONGGeometryShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(PHONGGeometryShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    //
    unsigned int PHONGFragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(PHONGFragShader, 1, &PHONGFragShaderSource.text, NULL);
    glCompileShader(PHONGFragShader);
    glGetShaderiv(PHONGFragShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(PHONGFragShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAG::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    //
    PHONGShaderProgram = glCreateProgram();
    glAttachShader(PHONGShaderProgram, PHONGVertexShader);
    glAttachShader(PHONGShaderProgram, PHONGGeometryShader);
    glAttachShader(PHONGShaderProgram, PHONGFragShader);

    glLinkProgram(PHONGShaderProgram);
    glGetProgramiv(PHONGShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(PHONGShaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FULLSHADERPROGRAM::LINK_FAILED\n" << infoLog << std::endl;
    }
    glUseProgram(PHONGShaderProgram);
    glEnable(GL_PROGRAM_POINT_SIZE);

    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error after PHONG shader: " << std::endl;
        return 1;
    }

    currentRenderProgram = PHONGShaderProgram;

    // load model data

    // const std::string path = "/home/gabrielnhn/cgv/SHREC_r/off_2/1.off";
    // const std::string path = "/home/gabrielnhn/cgv/SHREC_r/off_2/2.off";
    // const std::string path = "/home/gabrielnhn/cgv/SHREC_r/off_2/3.off";
    const std::string path = "/home/gabrielnhn/cgv/SHREC_r/off_2/4.off";
    
    std::cout << "READING: " << path << std::endl;
    
    // off_object = OffModel(path);
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

    glUseProgram(DepthShaderProgram);
    glUniform1f(glGetUniformLocation(DepthShaderProgram, "farPlaneDistance"), farDistance);
    // glUniform3f(glGetUniformLocation(DepthShaderProgram, "object_color"), 1.0f, 0.5f, 0.5f); 
    // glUniform3f(glGetUniformLocation(DepthShaderProgram, "ambient_light"), ambient_light.x,ambient_light.y,ambient_light.z); 

    glUseProgram(PHONGShaderProgram);
    // glUniform1f(glGetUniformLocation(PHONGShaderProgram, "farPlaneDistance"), farDistance);
    // glUniform3f(glGetUniformLocation(PHONGShaderProgram, "object_color"), 1.0f, 0.5f, 0.5f); 
    glUniform3f(glGetUniformLocation(PHONGShaderProgram, "ambient_light"), ambient_light.x,ambient_light.y,ambient_light.z); 
   

    // buffer model data to gpu
    glGenBuffers(1, &VBOPos);
    glGenBuffers(1, &VBOColor);

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);  
    glBindVertexArray(VAO);

    unsigned int EBO;
    glGenBuffers(1, &EBO);

    // VBOs FOR VERTICES
    //pos
    glBindBuffer(GL_ARRAY_BUFFER, VBOPos);  
    glBufferData(GL_ARRAY_BUFFER, off_object.vertices.size()*sizeof(glm::vec3), &off_object.vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, (void*)0);
    glEnableVertexAttribArray(0);  
    //color
    glBindBuffer(GL_ARRAY_BUFFER, VBOColor);  
    glBufferData(GL_ARRAY_BUFFER, off_object.features.size()*sizeof(glm::vec3), &off_object.features[0], GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, (void*)0);
    glEnableVertexAttribArray(1);  


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

        glUseProgram(0);
        glUseProgram(currentRenderProgram);

        if (should_reset)
        {
            reset_features(&off_object);
            should_reset = false;
        }

        // remake projection
        projection = glm::perspective(fov, aspect_ratio, nearDistance, farDistance);
        view = glm::lookAt(camera, aim, glm::vec3(0, 1, 0));
        mvp = projection * view * model;
        mv = view * model;

        // set variable shader uniforms
        int locationMVP = glGetUniformLocation(currentRenderProgram, "mvp");
        glUniformMatrix4fv(locationMVP, 1, GL_FALSE, glm::value_ptr(mvp));
        
        if (currentRenderProgram == DepthShaderProgram)
            glUniformMatrix4fv(glGetUniformLocation(currentRenderProgram, "mv"), 1, GL_FALSE, glm::value_ptr(mv));
        
        int locationLight = glGetUniformLocation(currentRenderProgram, "light_pos");
        glUniform3f(locationLight, camera.x, camera.y, camera.z);

        glBindVertexArray(VAO);

        // FOR POINT CLOUD
        // glPointSize(10.0f);
        // glDrawArrays(GL_POINTS, 0, off_object.vertices.size());

        // FOR SURFACES
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glDrawElements(GL_TRIANGLES, off_object.faces.size()*3, GL_UNSIGNED_INT, 0);

        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cerr << "OpenGL error after drawElements" << std::endl;
            return 1;
        }
        
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
            saveImage("./temp/depth.png", window, true);
            should_save_next_frame = false;
            unproject_image(
                projection,
                mv,
                // "",
                "./temp/depth.png",
                // window,
                &off_object, diag
            );
              
            currentRenderProgram = PHONGShaderProgram;
        }

        

        loop_count += 1;
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cerr << "OpenGL error: " << err << "at loop count " << loop_count << std::endl;
            return 1;
        }
       
    }

    glDeleteShader(DepthVertexShader);
    glDeleteShader(DepthFragShader); 
    glDeleteProgram(DepthShaderProgram);
    // glDeleteShader(DepthGeometryShader); 

    glDeleteShader(PHONGVertexShader);
    glDeleteShader(PHONGFragShader); 
    glDeleteProgram(PHONGShaderProgram);
    glDeleteShader(PHONGGeometryShader);
    
    textFinish();

    return 0;
}