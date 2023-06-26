#pragma once

#ifndef _PROPERTIES_H
#define _PROPERTIES_H

#include "pgr.h"
#include "camera.h"
#include <memory>

/// Simple material properties
struct Material 
{
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	float shininess;

	Material(const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float shininess);
	virtual ~Material();
};

/// Material using diffuse and specular maps
struct MaterialMap : Material 
{
	GLuint diffuseMap;
	GLuint specularMap;

	MaterialMap(const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float shininess, const std::string& diffuseMapPath, const std::string& specularMapPath);
	MaterialMap(const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float shininess, const std::string& diffuseMapPath);

	~MaterialMap();
};

/// Data structure corresponding to same light struct in the shader
struct GLSLLight 
{
	glm::vec3 ambient;
	uint32_t point;
	glm::vec3 diffuse;
	uint32_t spotlight;
	glm::vec3 specular;
	float cutOff;
	glm::vec3 position;
	float exponent;
	glm::vec3 direction;
	float constant;
	float linear;
	float quadratic;
	float __PADDING0__;
	float __PADDING1__;
};

/// Enum for light types
enum LightType
{
	LIGHT_NONE,
	LIGHT_DIRECTIONAL,
	LIGHT_POINT,
	LIGHT_SPOTLIGHT
};

/// Generic light properties
struct Light 
{
	LightType type;

	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

	Light();
	Light(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular);

	/// Get a pointer to light data in a format that is possible to upload to light UBO
	virtual GLSLLight* toDataPtr() const;
};

/// Directional light properties
struct DirectionalLight : Light
{
	DirectionalLight();
	DirectionalLight(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular);
	GLSLLight* toDataPtr() const override;
};

/// Point light properties
struct PointLight : Light 
{
	float constant;
	float linear;
	float quadratic;

	PointLight();
	PointLight(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant = 1.0f, float linear = 0.01f, float quadratic = 0.005f);
	/// Get a pointer to light data in a format that is possible to upload to light UBO
	GLSLLight* toDataPtr() const override;
};

/// Spotlight properties
struct SpotLight : Light 
{
	float cutOff;
	float exponent;
	float constant;
	float linear;
	float quadratic;

	SpotLight();
	SpotLight(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float cutOff, float exponent, float constant = 1.0f, float linear = 0.00001f, float quadratic = 0.00008f);
	/// Get a pointer to light data in a format that is possible to upload to light UBO
	GLSLLight* toDataPtr() const override;
};


#define REFRESH_TIME unsigned int(float(1000) / Camera::refreshRate) // ms

struct Fog 
{
protected:
	/// Time over which fog is animated when turned on and off
	static const uint32_t FOG_ANIMATION_TIME = 1000;
	/// Timer callback to animate the fog
	static void animationTimerCallback(int);
	/// Current fog object
	static Fog* active;

public:
	/// Fog color
	glm::vec3 color;
	/// Fot target density
	float density;
	/// For equation exponent
	float gradient;
	/// Fog state
	bool isEnabled;

	/// Fog actual density, used in animation
	float currentDensity;
	/// Density shift step
	float step;
	/// Fog actual visibility, used in animation
	bool isVisible;

	Fog(const glm::vec3& color, float density, float gradient);

	void enable();
	void disable();
};

#endif