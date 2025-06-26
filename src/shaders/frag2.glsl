#version 330 core

in vec2 TexCoord;
in int TexIndex;

out vec4 FragColor;

uniform sampler2D textures[12]; // Array of samplers

void main() {
    FragColor = texture(textures[TexIndex], TexCoord);
}