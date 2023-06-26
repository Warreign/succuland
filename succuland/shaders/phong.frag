#version 400 core

out vec4 fColor;

uniform struct Material
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;

	bool useDiffuseMap;
	bool useSpecularMap;
	sampler2D diffuseMap;
	sampler2D specularMap;
} material;

uniform struct Fog
{
	vec3 color;
	float density;
	float gradient;
	bool isEnabled;
} fog;

#define MAX_LIGHT_NUM 50

struct Light 
{
	vec3 ambient;
	bool point;
	vec3 diffuse;
	bool spotlight;
	vec3 specular;
	float cutOff;
	vec3 position;
	float exponent;
	vec3 direction;
	float constant;
	float linear;
	float quadratic;
}; 

layout (std140) uniform Lights
{
	uint lightNum;
	Light lights[MAX_LIGHT_NUM];
} light_block;

uniform vec3 cameraPos;

smooth in vec3 vPosition;
smooth in vec3 vNormal;
smooth in vec2 vTexCoord;
smooth in float vDist;

float getFogFactor(Fog fog, float fogCoordinate)
{
	float result = exp(-pow(fog.density * fogCoordinate, fog.gradient));
	result = 1.0 - clamp(result, 0.0, 1.0);
	return result;
}

vec4 calculateLight(Light light) 
{
	vec3 ret = vec3(0.0);

	vec3 L, R, V;

	if (light.point || light.spotlight)
	{
		L = normalize(light.position - vPosition);
	} 
	else 
	{
		L = normalize(light.direction);
	}
	R = reflect(-L, vNormal);
	V = normalize(cameraPos-vPosition);

	float diffFactor = max(dot(vNormal, L), 0.0);
	float specFactor = pow(max(dot(R, V), 0.0), material.shininess);

	vec3 diffM = vec3(1.0);
	vec3 specM = vec3(1.0);
	if (material.useDiffuseMap)
	{
		diffM = texture(material.diffuseMap, vTexCoord).rgb;
	}
	if (material.useSpecularMap)
	{
		specM = texture(material.specularMap, vTexCoord).rgb;
	}

	vec3 outAmbient = material.ambient * light.ambient;
	vec3 outDiffuse = material.diffuse * light.diffuse * diffM * diffFactor;
	vec3 outSpecular = material.specular * light.specular * specM * specFactor;

	ret = (outAmbient + outDiffuse + outSpecular);
	
	if (light.spotlight) {
		float spotAngleCos = max(0.0, dot(-L, light.direction));
		if (spotAngleCos > 0.80) {
			ret *= pow(spotAngleCos, light.exponent);
		}
		else {
			return vec4(0.0, 0.0, 0.0, 1.0); 
		}
	}

	if (light.point || light.spotlight) {
		float dist = distance(light.position, vPosition);
		float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);
		ret *= attenuation;
	}

	return vec4(ret, 1.0);
}

void main() 
{
	float globalAmbient = 0.4;

	fColor = vec4(material.ambient * globalAmbient, 0.0);
	for (uint i = 0; i < light_block.lightNum; ++i) {
		fColor += calculateLight(light_block.lights[i]);
	}

	if (fog.isEnabled) 
	{
		fColor = mix(fColor, vec4(fog.color, 1.0), getFogFactor(fog, vDist));
	}
}