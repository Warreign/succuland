#version 400 core

in vec3 aPosition;
in vec2 aTexCoord;

out vec2 vTexCoord;

uniform float time;

void main()
{
	gl_Position = vec4(aPosition, 1.0);

	float localTime = time * 0.00005;

	vec2 offset = vec2((floor(localTime) - localTime) * 7 + 1.0, 0.0);

	vTexCoord = aTexCoord + offset;
}