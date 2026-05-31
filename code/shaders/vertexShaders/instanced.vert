#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 3) in mat4 aInstanceMatrix;

uniform mat4 uniView;
uniform mat4 uniProjection;

out vec2 TexCoord;

void main()
{
    gl_Position = uniProjection * uniView * aInstanceMatrix * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}