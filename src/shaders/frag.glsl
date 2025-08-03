#version 400 core

in vec2 TexCoord;
flat in int TexIndex;

out vec4 FragColor;

uniform sampler2D textures[12];

void main() {
    FragColor = texture(textures[TexIndex], TexCoord);
}