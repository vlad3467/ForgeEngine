#version 330 core

out vec4 color;

in vec2 texCoord;
in vec3 normal;

uniform sampler2D texture0;
uniform sampler2D texture1;

uniform vec3 lightAmbient;
uniform vec3 lightColor;
uniform vec3 lightPos;


void main()
{
    color = mix(texture(texture0,texCoord), texture(texture1,texCoord), 0.5);
}