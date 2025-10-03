#version 400 core

in vec2 TexCoord;
flat in int TexIndex;

out vec4 FragColor;

uniform sampler2D textures[12];

void main() {
    if (TexIndex == -1) {
        FragColor = vec4(1.0f, 1.0f, 1.0f, 0.4f);
    } else if (TexIndex == 0) {
        FragColor = texture(textures[TexIndex], vec2(TexCoord.x, 1.0f - TexCoord.y));
    } else {
        FragColor = texture(textures[TexIndex], TexCoord);
    }

    // Discard fragments that are nearly transparent to avoid depth issues
    if (FragColor.a < 0.01) {
        discard;
    }
}