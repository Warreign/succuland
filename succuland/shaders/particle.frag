#version 400 core

out vec4 fColor;

uniform sampler2D particle;

in vec2 vTexCoord;

void main()
{
	fColor = texture(particle, vTexCoord);
}