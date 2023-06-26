#version 400 core

out vec4 fColor;

in vec3 vTexCoord;
smooth in float vDist;

uniform samplerCube skybox;

uniform struct Fog
{
	vec3 color;
	float density;
	float gradient;
	bool isEnabled;
} fog;


float getFogFactor(Fog fog, float fogCoordinate)
{
	float result = exp(-pow(fog.density * fogCoordinate, fog.gradient));
	result = 1.0 - clamp(result, 0.0, 1.0);
	return result;
}

void main() 
{
	fColor = texture(skybox, vTexCoord);
}

