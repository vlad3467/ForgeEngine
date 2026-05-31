#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texture;
layout (location = 2) in vec3 norm;

out vec2 texCoord;
out vec3 normal;

uniform mat4 uniModel;
uniform mat4 uniView;
uniform mat4 uniProjection;
uniform mat4 uniTraitor;

void main()
{
    gl_Position = uniProjection * uniView * uniModel * vec4(pos, 1.0);
    texCoord = vec2(texture.x, 1.0 - texture.y);
    normal = norm;
}