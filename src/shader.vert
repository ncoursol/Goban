#version 330 core

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec4 vertexColor;

out vec2 UV;
out vec4 fragColor;

uniform mat4 MVP;
uniform float movement;

void main() {
	gl_Position = MVP * vec4(vertexPos, 1);
	fragColor = vertexColor;
	UV = vec2(vertexUV.x + movement, vertexUV.y + movement);
};
