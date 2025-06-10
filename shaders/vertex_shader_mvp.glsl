#version 330 core
layout (location = 0) in vec4 vertexPosition;
uniform mat4 mvp;
out vec4 geomPos;
// out float vertexShade;
void main()
{
    // vertexShade = vertexPosition.z;  // Pass the position directly to the fragment shader for color
    gl_Position = mvp * vertexPosition;  // Apply MVP transformation
    geomPos = vertexPosition;
};
