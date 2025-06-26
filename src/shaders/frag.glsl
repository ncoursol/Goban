#version 330 core

in vec2 UV;
in vec2 UV_Text;
in vec4 stoneCol;

out vec4 FragColor;

uniform sampler2D grid_text;
uniform sampler2D wood_text;


uniform sampler2D text;
uniform vec3 textColor;

void main()
{
    vec4 gridColor = texture(grid_text, UV);
    vec4 woodColor = texture(wood_text, UV);
    vec4 tColor = texture(text, UV_Text);

    float alpha = gridColor.a * 1.0;
    FragColor = mix(woodColor, gridColor, alpha) + stoneCol + tColor;
}