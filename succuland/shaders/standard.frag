#version 400 core

//in vec3 color;
out vec4 fColor;

in vec3 vColor;
in vec3 vNormal;
in vec2 vTexCoord;
in float vDist;

uniform struct Material 
{
	sampler2D diffuseMap;
} material;

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
	fColor = texture(material.diffuseMap, vTexCoord);
	if (fog.isEnabled) 
	{
		fColor = mix(fColor, vec4(fog.color, 1.0), getFogFactor(fog, vDist));
	}
}
