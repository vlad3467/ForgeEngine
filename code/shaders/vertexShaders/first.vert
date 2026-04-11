#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texture;

out vec3 ourColor;
out vec2 texCoord;

uniform mat4 uniModel;
uniform mat4 uniView;
uniform mat4 uniPerspective;

void main()
{
    gl_Position = uniPerspective * uniView * uniModel * vec4(pos, 1.0);
    texCoord = vec2(texture.x, 1.0 - texture.y);
}