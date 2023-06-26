#include "properties.h"

/*
*	Material
*/

Material::Material(const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float shininess) 
	: ambient(ambient), diffuse(diffuse), specular(specular), shininess(shininess)
{
}

Material::~Material() 
{
}

MaterialMap::MaterialMap(const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float shininess, const std::string& diffuseMapPath, const std::string& specularMapPath)
	: Material(ambient, diffuse, specular, shininess),
	diffuseMap(pgr::createTexture(diffuseMapPath)),
	specularMap(pgr::createTexture(specularMapPath))
{
	glBindTexture(GL_TEXTURE_2D, diffuseMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glBindTexture(GL_TEXTURE_2D, specularMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);
}

MaterialMap::MaterialMap(const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float shininess, const std::string& diffuseMapPath)
	: Material(ambient, diffuse, specular, shininess),
	diffuseMap(pgr::createTexture(diffuseMapPath)),
	specularMap(0)
{
	glBindTexture(GL_TEXTURE_2D, diffuseMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);
}

MaterialMap::~MaterialMap()
{
	glDeleteTextures(1, &diffuseMap);
	if (specularMap)
		glDeleteTextures(1, &specularMap);
}

/*
*	Light
*/

Light::Light() 
{
}

Light::Light(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular) 
	: ambient(ambient), diffuse(diffuse), specular(specular), type(LIGHT_NONE) 
{
}

GLSLLight* Light::toDataPtr() const {
	throw std::runtime_error(std::string(__FILE__) + "data pointer query on light without type");
}

PointLight::PointLight()
{
}

PointLight::PointLight(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, float linear, float quadratic)
	: Light(ambient, diffuse, specular),
	constant(constant), linear(linear), quadratic(quadratic)
{
	type = LIGHT_POINT;
}

GLSLLight* PointLight::toDataPtr() const 
{
	GLSLLight* data = new GLSLLight;
	data->ambient = ambient;
	data->diffuse = diffuse;
	data->specular = specular;
	data->point = true;
	data->spotlight = false;
	data->constant = constant;
	data->linear = linear;
	data->quadratic = quadratic;
	return data;
}

DirectionalLight::DirectionalLight() 
{
}

DirectionalLight::DirectionalLight(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular) 
	: Light(ambient, diffuse, specular)
{
	type = LIGHT_DIRECTIONAL;
}

GLSLLight* DirectionalLight::toDataPtr() const 
{
	GLSLLight* data = new GLSLLight;
	data->ambient = ambient;
	data->diffuse = diffuse;
	data->specular = specular;
	data->point = false;
	data->spotlight = false;
	return data;
}

SpotLight::SpotLight() 
{
}

SpotLight::SpotLight(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float cutOff, float exponent, float constant, float linear, float quadratic) 
	: Light(ambient, diffuse, specular),
	cutOff(cutOff), exponent(exponent), constant(constant), linear(linear), quadratic(quadratic) 
{
	type = LIGHT_SPOTLIGHT;
}

GLSLLight* SpotLight::toDataPtr() const
{
	GLSLLight* data = new GLSLLight;
	data->ambient = ambient;
	data->diffuse = diffuse;
	data->specular = specular;
	data->point = false;
	data->spotlight = true;
	data->cutOff = cutOff;
	data->exponent = exponent;
	data->constant = constant;
	data->linear = linear;
	data->quadratic = quadratic;
	return data;
}

Fog* Fog::active = nullptr;

Fog::Fog(const glm::vec3& color, float density, float gradient)
	: color(color), density(density), gradient(gradient), isEnabled(false), currentDensity(0.001), isVisible(false), step(0)
{
	active = this;
}

void Fog::animationTimerCallback(int startTime)
{
	if (active->isEnabled) 
	{
		active->currentDensity += active->step;
	}
	else
	{
		active->currentDensity -= active->step;
	}

	if (glutGet(GLUT_ELAPSED_TIME) - startTime < FOG_ANIMATION_TIME)
	{
		glutTimerFunc(REFRESH_TIME, animationTimerCallback, startTime);
	}
	else
	{
		if (!active->isEnabled)
			active->isVisible = false;
	}
}

void Fog::enable()
{
	if (isEnabled)
		return;
	isEnabled = true;
	isVisible = true;
	step = (density - 0.001) / (FOG_ANIMATION_TIME / REFRESH_TIME);
	glutTimerFunc(REFRESH_TIME, animationTimerCallback, glutGet(GLUT_ELAPSED_TIME));
}

void Fog::disable()
{
	if (!isEnabled)
		return;
	isEnabled = false;
	step = (density - 0.001) / (FOG_ANIMATION_TIME / REFRESH_TIME);
	glutTimerFunc(REFRESH_TIME, animationTimerCallback, glutGet(GLUT_ELAPSED_TIME));
}
