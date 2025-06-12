#version 330 core
in vec3 normal;
in vec4 fragPos;
in vec3 fragColor;
out vec4 FragColor;

uniform sampler1D colorMap;
uniform int shouldComputeSimilarity;
uniform vec3 referenceValue;

void main()
{
    if (shouldComputeSimilarity > 0)
    {
        float similarity = dot(fragColor, referenceValue) / (length(fragColor)*length(referenceValue));
        // FragColor = vec4(vec3(similarity/2.0), 1.0);
        // float scaled = (similarity-0.5)*2;
        // vec3 color = texture(colorMap, scaled).rgb;

        vec3 color = texture(colorMap, similarity-0.25).rgb;
        FragColor = vec4(color, 1.0);
    }
    else
    {
        FragColor = vec4(fragColor, 1.0);
    }
    // FragColor = vec4(gl_FragCoord.z, intermediateFragColor.yzw);
};

 