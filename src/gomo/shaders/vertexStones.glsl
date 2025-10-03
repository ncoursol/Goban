#version 400 core

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec2 vertexUV;
layout(location = 3) in vec3 stonePos;
layout(location = 4) in vec3 stoneColor;

out vec2 TexCoord;
out vec3 TexColor;

uniform mat4 MVP;

// Simple pseudo-random function
float pseudoRandom(float seed) {
    return fract(sin(seed) * 10000.0);
}

void main() {
    vec3 pos = vertexPos + stonePos;

    gl_Position = MVP * vec4(pos, 1.0);
    TexCoord = vertexUV;
    TexColor = stoneColor;
}