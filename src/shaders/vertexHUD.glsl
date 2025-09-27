#version 330 core

layout (location = 5) in vec3 vertexPos;
layout (location = 6) in vec2 vertexUV;

out vec2 TexCoords;

uniform mat4 proj;
uniform vec3 playerPos;
uniform vec3 centerTextPos;

void main()
{
    if (centerTextPos.x != 0 || centerTextPos.y != 0 || centerTextPos.z != 0) {
        // Simple billboard that faces the camera
        vec3 toCamera = normalize(playerPos - centerTextPos);
        
        // Calculate rotation angle around Y axis to face camera
        float angle = atan(toCamera.x, toCamera.z);
        
        // Create rotation matrix around Y axis
        float c = cos(angle);
        float s = sin(angle);
        
        // Transform vertex position relative to text center
        vec3 local = vertexPos - centerTextPos;
        
        // Apply Y-axis rotation to face camera (no X flip)
        vec3 rotated;
        rotated.x = local.x * c + local.z * s;
        rotated.y = local.y;
        rotated.z = -local.x * s + local.z * c;
        
        vec3 worldPos = centerTextPos + rotated;
        
        gl_Position = proj * vec4(worldPos, 1.0);
    } else {   
        // Regular 2D text rendering (no billboard effect)
        gl_Position = proj * vec4(vertexPos, 1.0);
    }
    TexCoords = vertexUV;
}