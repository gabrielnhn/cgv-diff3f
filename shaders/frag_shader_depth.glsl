
#version 330 core
out vec4 FragColor;
in float vertexShade;

uniform float farPlaneDistance;  // Model-View-Projection matrix


void main()
{
    // FragColor = vec4(vec3(0.70 + (vertexShade/farPlaneDistance)), 1.0);
    // FragColor = vec4(vec3(1-gl_FragCoord.z), 1.0);
    FragColor = vec4(vec3(gl_FragCoord.z),1.0);
}