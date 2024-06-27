#version 330 core

layout (location = 5) in vec4 vertex;

out vec2 TexCoords;

uniform mat4 ortho;

void main()
{
    gl_Position = ortho * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}