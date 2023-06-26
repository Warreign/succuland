#version 400 core

out vec4 fColor;

in vec2 vTexCoord;

uniform sampler2D banner;

void main()
{
	fColor = texture(banner, vTexCoord);
}