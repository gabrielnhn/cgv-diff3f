#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include <cassert>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include <utility>
#include <filesystem>

#include "load_off_model.hpp"
#include "load_shader.hpp"
#include "save_image.hpp"
#include "load_image.hpp"
#include "draw_text.hpp"
// #include "run_python.hpp"

#include "features.hpp"

	

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

// float max_distance_to_object = 1.5;
float max_distance_to_object = 2.0;
auto default_camera = glm::vec3(0.0f, 0.0f, max_distance_to_object);
auto aim = glm::vec3(0.0f);
auto nearDistance = 0.1f;
// auto farDistance = 20.0f;
auto farDistance = 5.0f;
auto fov = glm::radians(60.0f);

double mousex, mousey;
double mousex_last, mousey_last;
// int last_mouse_event = GLFW_RELEASE;
float sensitivity = 10.0f;

float speed = 0.05f;

// for 1 window
// glm::mat4 mvp;
// glm::mat4 mv;

// float aspect_ratio = width/height;

// // image saving stuff
// bool should_save_next_frame = false;
// bool should_reset = false;

// unsigned int DepthShaderProgram;
// unsigned int PHONGShaderProgram;
// unsigned int currentRenderProgram = 0;

// unsigned int VBOPos, VBOColor;


std::vector<glm::vec3>cameras = {default_camera, default_camera};
std::vector<float> heights = {800, 800};
std::vector<float> widths = {800, 800};

std::vector<glm::mat4> mvps = {glm::mat4(1.0), glm::mat4(1.0)};
std::vector<glm::mat4> mvs = {glm::mat4(1.0), glm::mat4(1.0)};
std::vector<glm::mat4> models = {glm::mat4(1.0), glm::mat4(1.0)};
std::vector<glm::mat4> views = {glm::mat4(1.0), glm::mat4(1.0)};
std::vector<glm::mat4> projections = {glm::mat4(1.0), glm::mat4(1.0)};

std::vector<float> aspect_ratios = {widths[0]/heights[0], widths[1]/heights[1]};

// image saving stuff
std::vector<bool> should_save_next_frame = {false, false};
std::vector<bool> should_reset = {false, false};
std::vector<bool> should_compute_similarity = {false, false};

std::vector<unsigned int> DepthShaderPrograms = {0,0};
std::vector<unsigned int> PHONGShaderPrograms = {0,0};
std::vector<unsigned int> currentRenderPrograms = {0,0};
std::vector<unsigned int> VBOPos = {0,0};
std::vector<unsigned int> VBOColors = {0,0};

std::map<GLFWwindow*, int> windowToIndex;
std::map<int,GLFWwindow*> indexToWindow;

std::vector<unsigned int> VAOs = {0,0};
std::vector<unsigned int> EBOs = {0,0};
std::vector<GLFWwindow*> windows = {NULL, NULL};  

std::vector<int> last_mouse_events = {GLFW_RELEASE, GLFW_RELEASE};
std::vector<int> last_keydir_events = {GLFW_RELEASE, GLFW_RELEASE};

int currentFeatureComputer = 1;


std::vector<OffModel> objects = {OffModel(), OffModel()};
std::vector<float> diags = {0,0};
const std::string dataset_path = "./external/SHREC_r/off_2/";
int dataset_size;


// load models
int reload_models()
{
    for(unsigned long int i = 0; i < objects.size(); i++)
    {
        glfwMakeContextCurrent(windows[i]);

        // off_object = OffModel(path);
        // find model bounding box
        glm::vec3 bbMin( std::numeric_limits<float>::max());
        glm::vec3 bbMax(-std::numeric_limits<float>::max());
        
        for (auto &v: objects[i].vertices) {
            bbMin = glm::min(bbMin, v);
            bbMax = glm::max(bbMax, v);
        }
        
        glm::vec3 center = (bbMin + bbMax) * 0.5f;
        diags[i] = glm::length(bbMax - bbMin);
        
        // move and scale to fit bbox
        models[i] = glm::mat4(1.0f);
        models[i] = glm::translate(models[i], -center); 
        models[i] = glm::scale(models[i], glm::vec3(2.0f / diags[i]));
        
        // prepare mvp matrices
        projections[i] = glm::perspective(fov, aspect_ratios[i], nearDistance, farDistance);
        views[i] = glm::lookAt(cameras[i], aim, glm::vec3(0, 1, 0));
        mvps[i] = projections[i] * views[i] * models[i];
        mvs[i] = views[i] * models[i];
        
        // set persistent shader uniforms
        auto ambient_light = glm::vec3(0.3f, 0.3f, 0.3f);
        
        glUseProgram(DepthShaderPrograms[i]);
        glUniform1f(glGetUniformLocation(DepthShaderPrograms[i], "farPlaneDistance"), farDistance);
        // glUniform3f(glGetUniformLocation(DepthShaderProgram, "object_color"), 1.0f, 0.5f, 0.5f); 
        // glUniform3f(glGetUniformLocation(DepthShaderProgram, "ambient_light"), ambient_light.x,ambient_light.y,ambient_light.z); 
        
        glUseProgram(PHONGShaderPrograms[i]);
        // glUniform1f(glGetUniformLocation(PHONGShaderProgram, "farPlaneDistance"), farDistance);
        // glUniform3f(glGetUniformLocation(PHONGShaderProgram, "object_color"), 1.0f, 0.5f, 0.5f); 
        glUniform3f(glGetUniformLocation(PHONGShaderPrograms[i], "ambient_light"), ambient_light.x,ambient_light.y,ambient_light.z); 
        glUniform1i(glGetUniformLocation(PHONGShaderPrograms[i], "shouldComputeSimilarity"), 0); 
        
        
        // buffer model data to gpu
        // glGenBuffers(1, &VBOPos[i]);
        // glGenBuffers(1, &VBOColors[i]);
        
        // unsigned int VAO;
        // unsigned int EBO;
        glGenVertexArrays(1, &VAOs[i]);  
        glBindVertexArray(VAOs[i]);
        
        glGenBuffers(1, &EBOs[i]);
        
        // VBOs FOR VERTICES
        //pos
        glBindBuffer(GL_ARRAY_BUFFER, VBOPos[i]);  
        glBufferData(GL_ARRAY_BUFFER, objects[i].vertices.size()*sizeof(glm::vec3), &objects[i].vertices[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, (void*)0);
        glEnableVertexAttribArray(0);  
        //color
        glBindBuffer(GL_ARRAY_BUFFER, VBOColors[i]);  
        glBufferData(GL_ARRAY_BUFFER, objects[i].features.size()*sizeof(glm::vec3), &objects[i].features[0], GL_DYNAMIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, (void*)0);
        glEnableVertexAttribArray(1);  
        
        
        // EBO FOR FACES
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, objects[i].faces.size()*sizeof(glm::ivec3), &objects[i].faces[0], GL_STATIC_DRAW); 
    }
    return 1;
}


void framebuffer_size_callback(GLFWwindow* window, int w, int h)
{
    int i = windowToIndex[window];
    widths[i] = w;
    heights[i] = h;
    glfwMakeContextCurrent(window);
    glViewport(0, 0, w, h);
    std::cout << "CHANGED WINDOW SIZE to w,h" << w << ", " << h << std::endl;
    std::cout << aspect_ratios[i] << std::endl;
    aspect_ratios[i] = w/h;
    std::cout << aspect_ratios[i] << std::endl;

    // Update text projection matrix
    glUseProgram(textPrograms[i]);
    glm::mat4 textProjection = glm::ortho(0.0f, (float)w, 0.0f, (float)h);
    int projectionLocation = glGetUniformLocation(textPrograms[i], "projection");
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(textProjection));

     // prepare mvp matrices
    // projections[i] = glm::perspective(fov, aspect_ratios[i], nearDistance, farDistance);
    // views[i] = glm::lookAt(cameras[i], aim, glm::vec3(0, 1, 0));
    // mvps[i] = projections[i] * views[i] * models[i];
    // mvs[i] = views[i] * models[i];
    // glUseProgram(0);
}  

int updateModelVBO(OffModel* off_object, int windowIndex)
{
    glfwMakeContextCurrent(windows[windowIndex]);

    glBindBuffer(GL_ARRAY_BUFFER, VBOColors[windowIndex]);  
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


void reset_features(OffModel* off_object, int windowIndex)
{
    #pragma omp parallel for
    for(long unsigned int i = 0; i < off_object->features.size(); i++)
    {
        off_object->features[i] = off_object->default_feature;
        off_object->hits[i] = 0;
    }
    updateModelVBO(off_object, windowIndex);
}


// move camera position according to keyboard and stuff
void processInput(GLFWwindow *window)
{
    int i = windowToIndex[window];

    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    glm::vec3 forward = glm::normalize(aim - cameras[i]);
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
    glm::vec3 up = glm::normalize(glm::cross(forward, glm::vec3(1, 0, 0)));
    // up = glm::abs(up);

    // auto forward = glm::vec3(0,0,1);
    // auto right = glm::vec3(1,0,0);
    // auto up = glm::vec3(0,1,0);


    // keyboard
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameras[i] -= speed * up;
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameras[i] += speed * up;

    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameras[i] += speed * right;
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameras[i] -= speed * right;

    if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        cameras[i] += speed * forward;
    if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        cameras[i] -= speed * forward;

    // if(glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE)
    if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        cameras[0] = default_camera;
        should_reset[0] = true;
        cameras[1] = default_camera;
        should_reset[1] = true;
    }


    int leftKey = glfwGetKey(window, GLFW_KEY_LEFT); 
    int rightKey = glfwGetKey(window, GLFW_KEY_RIGHT); 

    if ((not last_keydir_events[i]) and (leftKey or rightKey))
    {
        int index = objects[i].datasetIndex;
        if (leftKey)
            index -= 1;
        else
            index += 1;
        
        // 1 to 44
        if(index < 1)
            index = dataset_size;
        if(index > dataset_size)
            index = 1;

        // std::string path = std::print(dataset_path, index);
        std::string path = dataset_path + std::to_string(index) + ".off";

        objects[i] = OffModel(path, index);
        reload_models();
        last_keydir_events[i] = GLFW_PRESS;
    }
    else if (not (leftKey or rightKey))
        last_keydir_events[i] = GLFW_RELEASE;


    
    glfwGetCursorPos(window, &mousex, &mousey);
    // only if mouse within window

    if (((0 <= mousex) and (mousex <= widths[i]))
        and ((0 <= mousey) and (mousey <= heights[i])))
    {   
        if((glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
        or (glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS))
        {
            should_save_next_frame[i] = true;
            currentRenderPrograms[i] = DepthShaderPrograms[i];
        }
        
        if((glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        or (glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS))
        {
            should_compute_similarity[i] = true;
            currentRenderPrograms[i] = DepthShaderPrograms[i];
        }
        else
            should_compute_similarity[i] = false;

    }
    else
    {
        should_compute_similarity[i] = false;
    }



    if(glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        // currentRenderPrograms[i] = PHONGShaderPrograms[i];
        currentFeatureComputer = 1;
    }
    if(glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        // currentRenderPrograms[i] = DepthShaderPrograms[i];
        currentFeatureComputer = 2;
    }
    if(glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
    {
        // currentRenderPrograms[i] = DepthShaderPrograms[i];
        currentFeatureComputer = 3;
    }
    // if(glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
    // {
    //     // currentRenderPrograms[i] = DepthShaderPrograms[i];
    //     currentFeatureComputer = 4;
    // }
    // if(glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
    // {
    //     // currentRenderPrograms[i] = DepthShaderPrograms[i];
    //     currentFeatureComputer = 5;
    // }

   
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
        last_mouse_events[i] = 0;
   
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        if (last_mouse_events[i] == 0)
        {
            mousex_last = mousex;
            mousey_last = mousey;
            last_mouse_events[i] = 1;           
        }
        else
        {   
            float xdiff = (mousex - mousex_last)/widths[i];
            float ydiff = (mousey - mousey_last)/heights[i];

            cameras[i] -= xdiff * right * sensitivity;
            cameras[i] -= ydiff * up * sensitivity;
            
            mousex_last = mousex;
            mousey_last = mousey;
        }
    }
    
    // clamp with radius=max_distance
    if (glm::length(cameras[i]) > max_distance_to_object)
        cameras[i] = glm::normalize(cameras[i]) * max_distance_to_object;

}

int unproject_image(glm::mat4 current_projection, glm::mat4 current_mv,
    std::string feature_image_path,
    std::string depth_image_path,
    GLFWwindow* window,
    OffModel* off_object, float diag)
{
    int i = windowToIndex[window];
    // float random_float1 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    // float random_float2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    // float random_float3 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

    myImage depth_image(depth_image_path);
    myImage feature_image(feature_image_path);

    // glfwGetCursorPos(window, &mousex, &mousey);
    // float x_feat = mousex;
    // float y_feat = mousey;
    glm::vec4 viewport(0, 0, widths[i], heights[i]);
    
    #pragma omp parallel for
    for (unsigned long int k = 0; k < off_object->vertices.size(); k++)
    {
        float small_noise = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        small_noise = small_noise / 10.0;

        glm::vec3 ndc = glm::project(off_object->vertices[k],
                                    current_mv,
                                    current_projection,
                                    viewport);
      

        if (ndc.x < 0 || ndc.x >= depth_image.width || ndc.y < 0 || ndc.y >= depth_image.height) continue;

        glm::vec3 feature;
        float depthBuf = depth_image.getValue(heights[i] - ndc.y,ndc.x).r;
        
        if (currentFeatureComputer == DEPTHMAGMA)
            feature = depth_image.toMagma(heights[i] - ndc.y, ndc.x);
        
        else
        {
            feature = feature_image.getValue(heights[i] - ndc.y, ndc.x);
        }
        

        float projDepth = ndc.z;

        // if (projDepth < depthBuf + diag*0.002)
        if (projDepth < depthBuf + diag*0.01)
        {
            off_object->hits[k] += 1;
            float weight = 1.0f / off_object->hits[k];
            auto prev_value = off_object->features[k] * (1.0f - weight);
            // auto new_value = glm::vec3(random_float1+small_noise, random_float2+small_noise, random_float3+small_noise) * weight;
            auto new_value = feature * weight;
            
            
            off_object->features[k] = prev_value + new_value;
        }
    }
    
    updateModelVBO(off_object, i);

    return 1;
}

int similarity_setup(glm::mat4 current_projection, glm::mat4 current_mv,
    // std::string feature_image_path,
    std::string depth_image_path,
    GLFWwindow* window,
    OffModel* off_object, float diag)
{
    (void)diag;
    float ZFACTOR = 5.0;
    // float ZFACTOR = 0.1;

    int i = windowToIndex[window];
    myImage depth_image(depth_image_path);

    glfwGetCursorPos(window, &mousex, &mousey);
    float x_feat = mousex;
    float y_feat = mousey;
    glm::vec4 viewport(0, 0, widths[i], heights[i]);
    
    // auto target = glm::vec3(x_feat, y_feat, depthBuf);
    float depthBuf = depth_image.getValue(heights[i] - y_feat, x_feat).r;
    auto target = glm::vec3(x_feat, heights[i] - y_feat, depthBuf * ZFACTOR);

    int minIndex = -1;
    float minDist = 99999.0f;
    glm::vec3 minNdc;
 
    // std::cout << "LOOKING FOR POINT " << std::endl;
    // #pragma omp parallel for reduction (min: minDist)
    for (unsigned long int k = 0; k < off_object->vertices.size(); k++)
    {

        glm::vec3 ndc = glm::project(off_object->vertices[k],
                                    current_mv,
                                    current_projection,
                                    viewport);
 
        ndc.z = ndc.z * ZFACTOR;
        float dist = glm::distance(ndc, target);
        // std::cout << "dist " << dist << std::endl;
        // std::cout << "depthBuf " << depthBuf << std::endl;
        // std::cout << "ndc.z " << ndc.z << std::endl;
        // if ((dist < minDist) and (ndc.z <= target.z + diag*0.005))
        if ((dist < minDist))
        {
            minDist = dist;
            minIndex = k;
            minNdc = ndc;
        }
    }
    // std::cout << "FOUND POINT " << ndc.x << ", " << ndc.y << ", " << ndc.z << ", " << std::endl;
    // off_object->features[k];
    // std::cout << "reference Point has index " << minIndex << std::endl;
    glfwMakeContextCurrent(indexToWindow[1-i]);
    glUseProgram(PHONGShaderPrograms[1-i]);
    glUniform1i(glGetUniformLocation(PHONGShaderPrograms[1-i], "shouldComputeSimilarity"), 1); 
    glUniform3f(glGetUniformLocation(PHONGShaderPrograms[1-i], "referenceValue"), off_object->features[minIndex].x, off_object->features[minIndex].y, off_object->features[minIndex].z); 
    
    
    // set origin vertex as ref in depth shader for i
    glfwMakeContextCurrent(indexToWindow[i]);
    glUseProgram(DepthShaderPrograms[i]);
    glUniform1i(glGetUniformLocation(DepthShaderPrograms[i], "shouldComputeSimilarity"), 1); 
    glUniform3f(glGetUniformLocation(DepthShaderPrograms[i], "comparedPoint"), minNdc.x, minNdc.y, minNdc.z); 


    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error after sim setup: " << err << std::endl;
        exit(-1);
    }

    return 1;
}

bool conda_successful = true;

int main(int argc, char* argv[])
{
    // (void)argc;
    // (void)argv;

    std::vector<std::string> args(argc);     
    for (int i = 0; i < argc; ++i){
        args[i] = argv[i];
        // std::cout << "arg " << i << "is " << args[i] << std::endl;
    }

    if ((argc>1) and (args[1].find("--noconda") != std::string::npos))
    {
        conda_successful = false;
        std::cout << "NOT using conda features. Most features are disabled!" << std::endl;
    }


    // run_python(argc, argv, "./src/diffusion.py");
    if ((conda_successful) and (not init_python(argc, argv)))
    {
        std::cout << "Python wasnt initialized correctly." << std::endl;
        return 1;
    }

    featureIndexToString[DINO] = "DINO";
    featureIndexToString[DEPTHMAGMA] = "Depth (Magma)";
    featureIndexToString[LBP] = "Local Binary Pattern";
    // feature();
    // return 0;

    GLenum err;

    // PREPARE WINDOW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
  
    GLFWwindow* windowFirst = glfwCreateWindow(widths[0], heights[0], "Diff3F Window 1", NULL, NULL);
    if (windowFirst == NULL)
    {
        std::cout << "Failed to create GLFW window..." << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(windowFirst);

    windowToIndex[windowFirst] = 0;
    indexToWindow[0] = windowFirst;



    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }   

    glViewport(0, 0, widths[0], heights[0]);
    glfwSetFramebufferSizeCallback(windowFirst, framebuffer_size_callback);

    glfwSetInputMode(windowFirst, GLFW_CURSOR, GLFW_CURSOR_NORMAL); 
    aspect_ratios[0] = widths[0]/heights[0];

    
    // GLFWwindow* windowOther = glfwCreateWindow(widths[1], heights[1], "Diff3F Window 2", NULL, NULL);
    GLFWwindow* windowOther = glfwCreateWindow(widths[1], heights[1], "Diff3F Window 2", NULL, windowFirst);
    if (windowOther == NULL)
    {
        std::cout << "Failed to create GLFW window..." << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwSetFramebufferSizeCallback(windowOther, framebuffer_size_callback);

    indexToWindow[1] = windowOther;
    windowToIndex[windowOther] = 1;

    windows = {windowFirst, windowOther};  

    

    // PREPARE SHADERS (2 shader programs per window = 4)
    for(unsigned long int i = 0; i < windows.size(); i++)
    {        
            
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
        DepthShaderPrograms[i] = glCreateProgram();
        glAttachShader(DepthShaderPrograms[i], DepthVertexShader);
        glAttachShader(DepthShaderPrograms[i], DepthFragShader);
        
        glLinkProgram(DepthShaderPrograms[i]);
        glGetProgramiv(DepthShaderPrograms[i], GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(DepthShaderPrograms[i], 512, NULL, infoLog);
            std::cerr << "ERROR::SHADER::FULLSHADERPROGRAM::LINK_FAILED\n" << infoLog << std::endl;
        }
        glUseProgram(DepthShaderPrograms[i]);
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
        PHONGShaderPrograms[i] = glCreateProgram();
        glAttachShader(PHONGShaderPrograms[i], PHONGVertexShader);
        glAttachShader(PHONGShaderPrograms[i], PHONGGeometryShader);
        glAttachShader(PHONGShaderPrograms[i], PHONGFragShader);
        
        glLinkProgram(PHONGShaderPrograms[i]);
        glGetProgramiv(PHONGShaderPrograms[i], GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(PHONGShaderPrograms[i], 512, NULL, infoLog);
            std::cerr << "ERROR::SHADER::FULLSHADERPROGRAM::LINK_FAILED\n" << infoLog << std::endl;
        }
        glfwMakeContextCurrent(windows[i]);
        glUseProgram(PHONGShaderPrograms[i]);
        glEnable(GL_PROGRAM_POINT_SIZE);

        // prepare viridis
        // Generate texture
        GLuint colorMapTex;
        glGenTextures(1, &colorMapTex);
        glBindTexture(GL_TEXTURE_1D, colorMapTex);
        //choose colormap
        // glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, (GLsizei)viridis.size(), 0, GL_RGB, GL_FLOAT, viridis.data());
        glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, (GLsizei)redblue.size(), 0, GL_RGB, GL_FLOAT, redblue.data());
        
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_1D, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_1D, colorMapTex);
        glUniform1i(glGetUniformLocation(PHONGShaderPrograms[i], "colorMap"), 0);
        
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cerr << "OpenGL error after PHONG shader: " << std::endl;
            return 1;
        }
        
        currentRenderPrograms[i] = PHONGShaderPrograms[i];
        
    }
    // load model data
    dataset_size = 0;
    for (const auto & entry : std::filesystem::directory_iterator(dataset_path)) {
        dataset_size += 1;
    }

    std::cout << "DATASET SIZE IS " << dataset_size << " (original is 44)" << std::endl;
    if (dataset_size < 1)
        std::cout << "Dataset is empty. Download SHREC_r and put it in ./external/" << std::endl;

    const std::string firstPath = dataset_path + std::to_string(1) + ".off";
    const std::string otherPath = dataset_path + std::to_string((int)dataset_size/2) +".off";
    
    std::cout << "READING: " << firstPath << std::endl;
    OffModel firstObject(firstPath, 1);
    std::cout << "READING: " << otherPath << std::endl;
    OffModel otherObject(otherPath, 22);
    
    

    // std::vector<OffModel> objects = {firstObject, otherObject};
    objects = {firstObject, otherObject};

    for(unsigned long int i = 0; i < objects.size(); i++)
    {
        glfwMakeContextCurrent(windows[i]);

        // off_object = OffModel(path);
        // find model bounding box
        glm::vec3 bbMin( std::numeric_limits<float>::max());
        glm::vec3 bbMax(-std::numeric_limits<float>::max());
        
        for (auto &v: objects[i].vertices) {
            bbMin = glm::min(bbMin, v);
            bbMax = glm::max(bbMax, v);
        }
        
        glm::vec3 center = (bbMin + bbMax) * 0.5f;
        diags[i] = glm::length(bbMax - bbMin);
        
        // move and scale to fit bbox
        models[i] = glm::mat4(1.0f);
        models[i] = glm::translate(models[i], -center); 
        models[i] = glm::scale(models[i], glm::vec3(2.0f / diags[i]));
        
        // prepare mvp matrices
        projections[i] = glm::perspective(fov, aspect_ratios[i], nearDistance, farDistance);
        views[i] = glm::lookAt(cameras[i], aim, glm::vec3(0, 1, 0));
        mvps[i] = projections[i] * views[i] * models[i];
        mvs[i] = views[i] * models[i];
        
        // set persistent shader uniforms
        auto ambient_light = glm::vec3(0.3f, 0.3f, 0.3f);
        
        glUseProgram(DepthShaderPrograms[i]);
        glUniform1f(glGetUniformLocation(DepthShaderPrograms[i], "farPlaneDistance"), farDistance);
        // glUniform3f(glGetUniformLocation(DepthShaderProgram, "object_color"), 1.0f, 0.5f, 0.5f); 
        // glUniform3f(glGetUniformLocation(DepthShaderProgram, "ambient_light"), ambient_light.x,ambient_light.y,ambient_light.z); 
        
        glUseProgram(PHONGShaderPrograms[i]);
        // glUniform1f(glGetUniformLocation(PHONGShaderProgram, "farPlaneDistance"), farDistance);
        // glUniform3f(glGetUniformLocation(PHONGShaderProgram, "object_color"), 1.0f, 0.5f, 0.5f); 
        glUniform3f(glGetUniformLocation(PHONGShaderPrograms[i], "ambient_light"), ambient_light.x,ambient_light.y,ambient_light.z); 
        glUniform1i(glGetUniformLocation(PHONGShaderPrograms[i], "shouldComputeSimilarity"), 0); 
        
        
        // buffer model data to gpu
        glGenBuffers(1, &VBOPos[i]);
        glGenBuffers(1, &VBOColors[i]);
        
        // unsigned int VAO;
        // unsigned int EBO;
        glGenVertexArrays(1, &VAOs[i]);  
        glBindVertexArray(VAOs[i]);
        
        glGenBuffers(1, &EBOs[i]);
        
        // VBOs FOR VERTICES
        //pos
        glBindBuffer(GL_ARRAY_BUFFER, VBOPos[i]);  
        glBufferData(GL_ARRAY_BUFFER, objects[i].vertices.size()*sizeof(glm::vec3), &objects[i].vertices[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, (void*)0);
        glEnableVertexAttribArray(0);  
        //color
        glBindBuffer(GL_ARRAY_BUFFER, VBOColors[i]);  
        glBufferData(GL_ARRAY_BUFFER, objects[i].features.size()*sizeof(glm::vec3), &objects[i].features[0], GL_DYNAMIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, (void*)0);
        glEnableVertexAttribArray(1);  
        
        
        // EBO FOR FACES
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, objects[i].faces.size()*sizeof(glm::ivec3), &objects[i].faces[0], GL_STATIC_DRAW); 
        
        glEnable(GL_DEPTH_TEST);
        
        // glClearColor(1.0f,1.0f,1.0f,1.0f);
        // glClearColor(ambient_light.x, ambient_light.y, ambient_light.z,1.0f);
        glClearColor(0.0f,0.0f,0.0f,0.0f);
        
        
        int text_loaded = textSetup(i);
        assert(text_loaded);
    }

    int loop_count = 0;   

    // while(true)
    while((not glfwWindowShouldClose(windowFirst)) and (not glfwWindowShouldClose(windowOther)))
    {
        for (long unsigned int i = 0; i < windows.size(); i++)
        {
            GLFWwindow* window = windows[i];
            glfwMakeContextCurrent(window);

           
        // while(!glfwWindowShouldClose(window))
        // {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            processInput(window); // recompute camera position
            
            glUseProgram(currentRenderPrograms[i]);
            
            if (should_reset[i])
            {
                reset_features(&objects[i], i);
                should_reset[i] = false;
            }
            
            // remake projection
            projections[i] = glm::perspective(fov, aspect_ratios[i], nearDistance, farDistance);
            views[i] = glm::lookAt(cameras[i], aim, glm::vec3(0, 1, 0));
            mvps[i] = projections[i] * views[i] * models[i];
            mvs[i] = views[i] * models[i];
            
            // set variable shader uniforms
            int locationMVP = glGetUniformLocation(currentRenderPrograms[i], "mvp");
            glUniformMatrix4fv(locationMVP, 1, GL_FALSE, glm::value_ptr(mvps[i]));
            
            if (currentRenderPrograms[i] == DepthShaderPrograms[i])
            glUniformMatrix4fv(glGetUniformLocation(currentRenderPrograms[i], "mv"), 1, GL_FALSE, glm::value_ptr(mvs[i]));
            
            int locationLight = glGetUniformLocation(currentRenderPrograms[i], "light_pos");
            glUniform3f(locationLight, cameras[i].x, cameras[i].y, cameras[i].z);
            
            glBindVertexArray(VAOs[i]);
            glBindBuffer(GL_ARRAY_BUFFER, VBOPos[i]);  
            glBindBuffer(GL_ARRAY_BUFFER, VBOColors[i]);  
            
            // FOR POINT CLOUD
            // glPointSize(10.0f);
            // glDrawArrays(GL_POINTS, 0, off_object.vertices.size());
            
            // FOR SURFACES
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[i]);
            // glDrawElements(GL_TRIANGLES, firstObject.faces.size()*3, GL_UNSIGNED_INT, 0);
            glDrawElements(GL_TRIANGLES, objects[i].faces.size()*3, GL_UNSIGNED_INT, 0);
            
            while ((err = glGetError()) != GL_NO_ERROR) {
                std::cerr << "OpenGL error " << err << " after drawElements" << std::endl;
                return 1;
            }
            
            // render text
            if (should_compute_similarity[i] or should_compute_similarity[1-i])
            {
                RenderText(i, "Red: Positive similarity. | Blue: Negative similarity", 25.0f, 770.0f, 0.5f, glm::vec3(0.5, 0.8f, 0.2f));
            }
            else if (not should_save_next_frame[i])
            {
                int text_rendered = RenderText(i, "[Mouse RClick]: Get features [Mouse Middle Click]: Compare similarity", 25.0f, 20.0f, 0.5f, glm::vec3(0.5, 0.8f, 0.2f));
                text_rendered = RenderText(i, "[R]: Reset                           [Mouse LClick]: Hold to drag model", 25.0f, 50.0f, 0.5f, glm::vec3(0.5, 0.8f, 0.2f));
                text_rendered = RenderText(i, "[1, 2, 3]: Change feature computing method", 25.0f, 770.0f, 0.5f, glm::vec3(0.5, 0.8f, 0.2f));
                text_rendered = RenderText(i, "[KeybLeft <= or KeybRight =>]: Previous/Next model (" + std::to_string(objects[i].datasetIndex) + ")", 25.0f, 740.0f, 0.5f, glm::vec3(0.5, 0.8f, 0.2f));
                text_rendered = RenderText(i, "Current method: " + featureIndexToString[currentFeatureComputer], 25.0f, 710.0f, 0.5f, glm::vec3(0.5, 0.8f, 0.2f));
                assert(text_rendered);
                // assert(text_rendered);

                glUseProgram(PHONGShaderPrograms[i]);
                glUniform1i(glGetUniformLocation(PHONGShaderPrograms[i], "shouldComputeSimilarity"), 0); 
            }
            
            glfwSwapBuffers(window);
            glfwPollEvents();
            
            if (should_save_next_frame[i])
            {
                saveImage("./temp/depth.png", window, true);
                feature(argc, argv, currentFeatureComputer, conda_successful);
                should_save_next_frame[i] = false;
                unproject_image(
                    projections[i],
                    mvs[i],
                    "./temp/feature.png",
                    "./temp/depth.png",
                    window,
                    &objects[i], diags[i]
                );
                
                currentRenderPrograms[i] = PHONGShaderPrograms[i];
            }
            
            if (should_compute_similarity[i])
            {
                // should_compute_similarity[i] = 0;
                similarity_setup(
                    projections[i],
                    mvs[i],
                    // "",
                    "./temp/depth.png",
                    window,
                    &objects[i], diags[i]
                );
                currentRenderPrograms[i] = PHONGShaderPrograms[i];
            }
            
            
            loop_count += 1;
            while ((err = glGetError()) != GL_NO_ERROR) {
                std::cerr << "OpenGL error: " << err << "at loop count " << loop_count << std::endl;
                return 1;
            }
            
        }
    }
        
    // glDeleteShader(DepthVertexShaders);
    // glDeleteShader(DepthFragShaders); 
    // glDeleteProgram(DepthShaderPrograms[0]); 
    // glDeleteShader(PHONGVertexShaders[0]);
    // glDeleteShader(PHONGFragShaders[0]); 
    // glDeleteProgram(PHONGShaderPrograms[0]);
    // glDeleteShader(PHONGGeometryShaders[0]);
    
    textFinish();
    if (conda_successful)
        finish_python();

    return 0;
}