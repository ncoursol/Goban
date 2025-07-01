#version 400 core

in vec2 TexCoord;
flat in int TexIndex;

out vec4 FragColor;

uniform sampler2D textures[12];

void main() {
    vec3 color;
    if (TexIndex == 0)      color = vec3(1.0, 0.0, 0.0);      // Red
    else if (TexIndex == 1) color = vec3(0.0, 1.0, 0.0);      // Green
    else if (TexIndex == 2) color = vec3(0.0, 0.0, 1.0);      // Blue
    else if (TexIndex == 3) color = vec3(1.0, 1.0, 0.0);      // Yellow
    else if (TexIndex == 4) color = vec3(1.0, 0.0, 1.0);      // Magenta
    else if (TexIndex == 5) color = vec3(0.0, 1.0, 1.0);      // Cyan
    else if (TexIndex == 6) color = vec3(1.0, 0.5, 0.0);      // Orange
    else if (TexIndex == 7) color = vec3(0.5, 0.0, 1.0);      // Purple
    else if (TexIndex == 8) color = vec3(0.5, 1.0, 0.0);      // Lime
    else if (TexIndex == 9) color = vec3(0.0, 0.5, 1.0);      // Sky Blue
    else if (TexIndex == 10) color = vec3(1.0, 0.0, 0.5);     // Pink
    else if (TexIndex == 11) color = vec3(0.3, 0.3, 0.3);     // Gray
    else color = vec3(1.0, 1.0, 1.0);                         // Default: White
    // FragColor = vec4(color, 1.0);

    FragColor = texture(textures[TexIndex], TexCoord);
}