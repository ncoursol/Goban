#version 330 core

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec4 vertexColor;
layout(location = 3) in vec3 vertexTest;

out vec2 UV;
out vec4 fragColor;

uniform mat4 MVP;

void main() {
	gl_Position = MVP * vec4(vertexPos + vertexTest, 1);
	fragColor = vec4(vertexTest, 1.0f);
	UV = vertexUV;
};
