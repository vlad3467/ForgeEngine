#version 330 core

in vec2 texCoord;

out vec4 color;

uniform sampler2D textTexture;
uniform vec3 textColor;

void main()
{
    float alpha = texture(textTexture, vec2(texCoord.x, 1 - texCoord.y)).r;
    color = vec4(textColor, alpha);
}