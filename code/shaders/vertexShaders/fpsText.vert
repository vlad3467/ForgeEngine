#version 330 core

layout (location = 0) in vec4 vertex;

out vec2 texCoord;

uniform mat4 uniProjection;

void main()
{
    gl_Position = uniProjection * vec4(vertex.xy, 0.0, 1.0);
    texCoord = vertex.zw;
}