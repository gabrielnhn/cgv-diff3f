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
        // AVOID ZERO COSINE SIM
        float epsilon = 0.0001; // Small constant to prevent division by zero
        float magnitudeFragColor = max(length(fragColor), epsilon);
        float magnitudeReferenceValue = max(length(referenceValue), epsilon);
        float similarity = dot(fragColor, referenceValue) / (magnitudeFragColor * magnitudeReferenceValue);
     
        vec3 color = texture(colorMap, similarity-0.25).rgb;
        FragColor = vec4(color, 1.0);
    }
    else
    {
        FragColor = vec4(fragColor, 1.0);
    }
};

 