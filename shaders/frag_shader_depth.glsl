
#version 330 core
out vec4 FragColor;
in float vertexShade;

uniform float farPlaneDistance;  // Model-View-Projection matrix


void main()
{
    // fragmentdepth = gl_FragCoord.z;
    // FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    // FragColor = vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, 1.0);
    // FragColor = vec4(vertexShade*0.05, vertexShade*0.05, vertexShade*0.05, 1.0);
    // FragColor = vec4(vertexShade, vertexShade, vertexShade, 1.0);
    FragColor = vec4(vec3(0.65 - (vertexShade/farPlaneDistance)), 1.0);
    // FragColor = vec4(1-vertexShade, 1-vertexShade, 1-vertexShade, 1.0);
}