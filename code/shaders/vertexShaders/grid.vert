#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 uniModel;
uniform mat4 uniView;
uniform mat4 uniProjection;

void main() {
    gl_Position = uniProjection * uniView * uniModel * vec4(aPos, 1.0);
}