#version 330 core

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec4 vertexColor;
layout(location = 3) in vec3 stonePos;
layout(location = 4) in vec3 stoneColor;
layout(location = 5) in vec4 textPos; // x, y, uvx, uvy

out vec2 UV;
out vec2 UV_Text;
out vec4 stoneCol;

uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(vertexPos + stonePos + vec3(textPos.xy, 0.0), 1.0);
	stoneCol = vec4(stoneColor, 1.0f);
	UV = vertexUV;
	UV_Text = vec2(textPos.z, textPos.w);
};