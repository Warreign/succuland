#version 400 core

in vec3 aPosition;
in vec2 aTexCoord;

uniform mat4 PVM;
uniform float time;

uniform ivec2 pattern = ivec2(7, 2);
vec2 offset = vec2(1.0) / vec2(pattern.x, -pattern.y);

vec2 texCoordBase = (aTexCoord / vec2(pattern)) + vec2(0.0, 0.5);

//float localTime = time * 0.0001;
//int frames = pattern.x * pattern.y;

//uniform float frameDuration = 0.1f;
uniform int frame;

out vec2 vTexCoord;

vec2 getTexCoord(int frame)
{
	return texCoordBase + vec2(frame % pattern.x, frame / pattern.x) * offset;
}

void main()
{
	gl_Position = PVM * vec4(aPosition, 1.0);

	vTexCoord = texCoordBase + vec2(frame % pattern.x, frame / pattern.x) * offset;
}