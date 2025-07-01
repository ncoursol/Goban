#version 330 core
layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in float texIndex;

out vec2 TexCoord;
flat out int TexIndex;

uniform mat4 MVP;

void main() {
	gl_Position = MVP * vec4(vertexPos, 1.0);
	TexCoord = vertexUV;
	TexIndex = (int)texIndex;
}