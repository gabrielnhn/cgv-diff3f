#version 330 core
in vec3 normal;
in vec4 fragPos;
uniform vec3 ambient_light;
uniform vec3 light_pos;
out vec4 FragColor;
void main()
{
    vec3 object_color = vec3(0.3, 0.5, 0.3);
    vec3 light_dir = normalize(light_pos - fragPos.xyz); // Corrected light direction calculation
    vec3 ambient = ambient_light * object_color;
    float diffuse_ang = max(0.0f, dot(normal, light_dir));
    vec3 diffuse = diffuse_ang * object_color;
    FragColor = vec4((ambient + diffuse)*object_color, 1.0);
};