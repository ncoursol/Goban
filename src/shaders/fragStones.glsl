#version 400 core

in vec2 TexCoord;
in vec3 TexColor;

out vec4 FragColor;

void main() {
    FragColor = vec4(TexColor, 1.0f);
}