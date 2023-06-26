#version 400 core	

in vec3 aPosition;

out vec3 vTexCoord;
smooth out float vDist;

uniform mat4 ProjectM;
uniform mat4 ViewM;
uniform mat4 ModelM;

void main() 
{
	vTexCoord = vec3(aPosition.x, -aPosition.y, aPosition.z);
	gl_Position = (ProjectM * mat4(mat3(ViewM)) * ModelM * vec4(aPosition, 1.0)).xyww;

	vDist = -(ModelM * vec4(aPosition, 1.0)).y * 20;
}