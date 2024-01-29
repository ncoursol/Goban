#version 330 core

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec4 vertexColor;
layout(location = 3) in vec3 stonePos;
layout(location = 4) in vec3 stoneColor;

out vec2 UV;
out vec4 stoneCol;

uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(vertexPos + stonePos, 1.0);
	stoneCol = vec4(stoneColor, 1.0f);
	UV = vertexUV;
};