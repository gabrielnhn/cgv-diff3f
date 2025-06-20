#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec4 geomPos[];
in vec3 geomColor[];
out vec3 normal;
out vec4 fragPos;
out vec3 fragColor;

uniform mat4 mvp;

void main() {
    vec3 v1 = geomPos[0].xyz;
    vec3 v2 = geomPos[1].xyz;
    vec3 v3 = geomPos[2].xyz;

    vec3 edge1 = v2 - v1;
    vec3 edge2 = v3 - v1;
    vec3 triangleNormal = normalize(cross(edge1, edge2));

    for (int i = 0; i < 3; i++) {
        normal = triangleNormal;
        fragPos = geomPos[i];
        fragColor = geomColor[i];
        gl_Position = mvp * geomPos[i];

        EmitVertex();
    }
    EndPrimitive();
};