#version 330 core

layout (location = 5) in vec3 vertexPos;
layout (location = 6) in vec2 vertexUV;

out vec2 TexCoords;

uniform mat4 proj;
uniform vec3 playerPos;
uniform vec3 cornerTextPos;
uniform vec3 textRotation; // Euler angles in radians (x, y, z)
uniform int faceCamera;    // Boolean: 1 = face camera, 0 = use custom rotation

mat3 rotationMatrix(vec3 euler) {
    float cx = cos(euler.x);
    float sx = sin(euler.x);
    float cy = cos(euler.y);
    float sy = sin(euler.y);
    float cz = cos(euler.z);
    float sz = sin(euler.z);
    
    // Combined rotation matrix (Z * Y * X order)
    mat3 rotX = mat3(1.0, 0.0, 0.0,
                     0.0, cx, -sx,
                     0.0, sx, cx);
    
    mat3 rotY = mat3(cy, 0.0, sy,
                     0.0, 1.0, 0.0,
                     -sy, 0.0, cy);
    
    mat3 rotZ = mat3(cz, -sz, 0.0,
                     sz, cz, 0.0,
                     0.0, 0.0, 1.0);
    
    return rotZ * rotY * rotX;
}

void main()
{
    vec3 worldPos = vertexPos;
    
    if (cornerTextPos.x != 0.0 || cornerTextPos.y != 0.0 || cornerTextPos.z != 0.0) {
        // 3D text rendering
        vec3 textCenter = cornerTextPos;
        vec3 local = vertexPos - textCenter;
        
        if (faceCamera == 1) {
            // Billboard effect: rotate text to face the camera
            vec3 toCamera = normalize(playerPos - textCenter);
            
            // Calculate rotation angle around Y axis to face camera
            float angle = atan(toCamera.x, toCamera.z);
            
            // Create rotation matrix around Y axis
            float c = cos(angle);
            float s = sin(angle);
            
            // Apply Y-axis rotation to face camera
            vec3 rotated;
            rotated.x = local.x * c + local.z * s;
            rotated.y = local.y;
            rotated.z = -local.x * s + local.z * c;
            
            worldPos = textCenter + rotated;
        } else if (faceCamera == 0) {
            // Apply custom rotation
            mat3 rotMat = rotationMatrix(textRotation);
            vec3 rotated = rotMat * local;
            worldPos = textCenter + rotated;
        } else if (faceCamera == 2) {
            // Apply custom rotation with text starting at cornerTextPos (not centered)
            // Rotate around the starting position
            vec3 startPos = cornerTextPos;
            vec3 local = vertexPos - startPos;
            mat3 rotMat = rotationMatrix(textRotation);
            vec3 rotated = rotMat * local;
            worldPos = startPos + rotated;
        }
        
        gl_Position = proj * vec4(worldPos, 1.0);
    } else {   
        // Regular 2D text rendering (no rotation)
        gl_Position = proj * vec4(vertexPos, 1.0);
    }
    TexCoords = vertexUV;
}