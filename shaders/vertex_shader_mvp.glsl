#version 330 core

layout (location = 0) in vec4 vertexPosition; // Expecting vec4 for each vertex
uniform mat4 mvp;  // Model-View-Projection matrix
out float vertexShade;  // Output color to fragment shader
out vec4 geomPos;
void main()
{
    vertexShade = vertexPosition.z;  // Pass the position directly to the fragment shader for color
    // gl_Position = mvp * vec4(vertexPosition, 1.0);  // Apply MVP transformation
    gl_Position = mvp * vertexPosition;  // Apply MVP transformation
    geomPos = vertexPosition;
};
