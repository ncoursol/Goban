#version 400 core

in vec2 TexCoord;
in vec3 TexColor;

out vec4 FragColor;

void main() {
    vec3 color = TexColor;
    float alpha = 1.0;

    FragColor = vec4(color, alpha);
}