
#version 330 core
out vec4 FragColor;
in float vertexShade;

uniform float farPlaneDistance;  // Model-View-Projection matrix

uniform vec3 comparedPoint;
uniform int shouldComputeSimilarity;

void main()
{
    if ((shouldComputeSimilarity > 0) && (distance(comparedPoint, gl_FragCoord.xyz) < 10.1))
    {
        FragColor = vec4(gl_FragCoord.z, 0.0, 0.0, 1.0);
    }
    else
    {
        FragColor = vec4(vec3(gl_FragCoord.z),1.0);
    }
}