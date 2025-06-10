#version 330 core

layout (location = 0) in vec3 vertexPosition; // Expecting vec4 for each vertex
uniform mat4 mvp;  // Model-View-Projection matrix
// uniform mat4 depthmvp;  // Model-View-Projection matrix
uniform mat4 mv;  // Model-View-Projection matrix
out float vertexShade;  // Output color to fragment shader
void main()
{
    gl_Position = mvp * vec4(vertexPosition, 1.0);  // Apply MVP transformation

    vec4 shadePosition = mv * vec4(vertexPosition, 1.0); 
    // vertexShade = -shadePosition.z;
    vertexShade = -shadePosition.z;
};
