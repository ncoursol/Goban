#version 330 core

layout (location = 5) in vec3 vertexPos;
layout (location = 6) in vec2 vertexUV;

out vec2 TexCoords;

uniform mat4 proj;
uniform vec3 playerPos;
uniform vec3 cornerTextPos;

void main()
{
    if (cornerTextPos.x != 0 || cornerTextPos.y != 0 || cornerTextPos.z != 0) {
        // Billboard effect: rotate text to face the camera
        vec3 textCenter = cornerTextPos;  // This is the center of the text
        vec3 toCamera = normalize(playerPos - textCenter);
        
        // Calculate rotation angle around Y axis to face camera
        float angle = atan(toCamera.x, toCamera.z);
        
        // Create rotation matrix around Y axis
        float c = cos(angle);
        float s = sin(angle);
        
        // Transform vertex position relative to text center
        vec3 local = vertexPos - textCenter;
        
        // Apply Y-axis rotation to face camera
        vec3 rotated;
        rotated.x = local.x * c + local.z * s;
        rotated.y = local.y;
        rotated.z = -local.x * s + local.z * c;
        
        // Transform back to world space
        vec3 worldPos = textCenter + rotated;
        
        gl_Position = proj * vec4(worldPos, 1.0);
    } else {   
        // Regular 2D text rendering (no billboard effect)
        gl_Position = proj * vec4(vertexPos, 1.0);
    }
    TexCoords = vertexUV;
}