#version 330 core

in vec2 TexCoord;
out vec4 color;

uniform sampler2D Tex;

void main()
{    
  color = texture2D(Tex,TexCoord);
}