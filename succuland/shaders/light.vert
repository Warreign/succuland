#version 400 core

const float grad = 2.0;

in vec3 aPosition;

smooth out float vDist;

uniform mat4 PVM;
uniform mat4 ModelM;
uniform vec3 cameraPos;

void main()
{
	gl_Position = PVM * vec4(aPosition, 1.0f);
	vDist = distance((ModelM * vec4(aPosition, 1.0)).xyz, cameraPos);
	
}