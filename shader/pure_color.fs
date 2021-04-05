#version 330 core

in vec2 TexCoord;
out vec4 OutputColor;

uniform vec4 color;

void main()
{
  OutputColor = color;
}