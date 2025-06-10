
#version 330 core
out vec4 FragColor;
in float vertexShade;
void main()
{
    // fragmentdepth = gl_FragCoord.z;
    // FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    // FragColor = vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, 1.0);
    FragColor = vec4(vertexShade+0.25, vertexShade+0.25, vertexShade+0.25, 1.0);
    // Not really needed, OpenGL does it anyway
}