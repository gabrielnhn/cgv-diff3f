#version 330 core

layout (location = 0) in vec3 vertexPosition;
uniform mat4 mvp; 
uniform mat4 mv;
out float vertexShade;

void main()
{
    // actual position to draw
    gl_Position = mvp * vec4(vertexPosition, 1.0);

    // postiion to apply depth map
    vec4 shadePosition = mv * vec4(vertexPosition, 1.0); 
    vertexShade = -shadePosition.z;
};
