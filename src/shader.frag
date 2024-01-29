#version 330 core

in vec2 UV;
in vec4 stoneCol;

out vec4 FragColor;

uniform sampler2D grid_text;
uniform sampler2D wood_text;

void main()
{
    vec4 gridColor = texture(grid_text, UV);
    vec4 woodColor = texture(wood_text, UV);

    float alpha = gridColor.a * 1.0;
    FragColor = mix(woodColor, gridColor, alpha) + stoneCol;
}
