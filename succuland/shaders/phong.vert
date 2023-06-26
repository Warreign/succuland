#version 400 core

in vec3 aPosition;
in vec3 aColor;
in vec3 aNormal;
in vec2 aTexCoord;

uniform mat4 PVM;
uniform mat4 ViewM;
uniform mat4 ModelM;
uniform mat4 NormalM;
uniform vec3 cameraPos;

smooth out vec3 vPosition;
smooth out vec3 vNormal;
smooth out vec2 vTexCoord;
smooth out float vDist;

void main() 
{
	gl_Position = PVM * vec4(aPosition, 1.0);
	vPosition = (ModelM * vec4(aPosition, 1.0)).xyz;
	vNormal = (NormalM * vec4(aNormal, 1.0)).xyz;;
	vTexCoord = aTexCoord;
	vDist = distance(cameraPos, vPosition);
}