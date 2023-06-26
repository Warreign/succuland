#version 400 core

in vec3 aPosition;
in vec3 aColor;
in vec3 aNormal;
in vec2 aTexCoord;

out vec3 vColor;
out vec3 vNormal;
out vec2 vTexCoord;
out float vDist;

uniform mat4 PVM;
uniform mat4 ModelM;
uniform vec3 cameraPos;

void main() 
{
	vColor = aColor;
	vTexCoord = aTexCoord;
	vNormal = aNormal;

	gl_Position = PVM * vec4(aPosition, 1.0f);
	vDist = distance((ModelM * vec4(aPosition, 1.0)).xyz, cameraPos);
}
