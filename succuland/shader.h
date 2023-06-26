#pragma once

#ifndef _SHADER_H
#define _SHADER_H

#include "pgr.h"
#include <iostream>

class Camera;
struct Material;
struct Light;
struct PointLight;
struct DirectionalLight;
struct SpotLight;
struct Fog;

/// UBO wrapper
class UniformBufferObject
{
protected:
	/// UBO handle
	GLuint bufferID;
	/// Size of corresponding block in shader
	size_t byteSize;

public:

	UniformBufferObject();
	/// <summary>
	/// Initialize UBO
	/// </summary>
	/// <param name="byteSize">Size of corresponding block in shader</param>
	/// <param name="bindingPoint">Binding point of the UBO</param>
	/// <param name="usageHint">Hint how to store data, defaults to GL_STREAM_DRAW</param>
	UniformBufferObject(const size_t byteSize, GLuint bindingPoint, GLenum usageHint = GL_STREAM_DRAW);

	~UniformBufferObject();

	/// <summary>
	/// Set data
	/// </summary>
	/// <param name="offset">Offset in the blcok</param>
	/// <param name="data">data pointer</param>
	/// <param name="dataSize">size of the data</param>
	void setData(const size_t offset, const void* data, const size_t dataSize);
};

/// Generic shader object
class Shader
{
protected:
	/// Program handle
	GLuint program;
	
public:
	/// Fog object, shared between all shaders
	static Fog* fog;
	/// Unbind currently bound shader program
	static void unbind();
	/// Change fog object
	static void setFog(Fog* fog);

	/// Shader attribute locations
	struct Attributes {
		GLint position;
		GLint color;
		GLint normal;
		GLint texCoord;
	} attributes;

	/// Shader uniform locations
	struct Uniforms {
		GLint PVM;
		GLint ViewM;
		GLint ModelM;
		GLint ProjectM;
	} uniforms;

	Shader();
	Shader(std::string vertexFile, std::string fragmentFile);
	virtual ~Shader() {}

	/// Use current shader
	void use() const;
	/// Delete current shader
	void clear();
	/// Load fog info to uniforms
	void loadFog() const;
	/// Set material uniforms
	virtual void setMaterial(Material* material) const;
	/// Set transform uniforms
	virtual void setTransformParameters(const Camera& camera, const glm::mat4& model) const;

	/// Set integer uniform
	void setInteger(const std::string uniformName, int value) const;
	/// Set float uniform
	void setFloat(const std::string uniformName, float value) const;
	/// Set vec2 uniform
	void setVec2(const std::string uniformName, glm::vec2& value) const;
	/// Set vec3 uniform
	void setVec3(const std::string uniformName, glm::vec3& value) const;
	/// Set vec4 uniform
	void setMat4(const std::string uniformName, glm::mat4& value) const;
};

/// Shader handling light
class LightingShader : public Shader 
{
protected:
	/// Maximum number of light objects UBO can handle
	static const int MAX_LIGHT_NUM = 50;
	/// Binding point of light UBO
	static const int LIGHTS_BINDING_POINT = 2;

	/// Current number of lights in UBO
	unsigned int lightsLoadedNum;
public:
	/// Shader uniform locations
	struct Uniforms {
		GLint PVM;
		GLint ViewM;
		GLint ModelM;
		GLint NormalM;
		GLint ProjectM;
		GLint cameraPos;

		GLint materialAmbient;
		GLint materialDiffuse;
		GLint materialSpecular;
		GLint materialShininess;
		GLint materialDiffuseMap;
		GLint materialSpecularMap;
		GLint materialNormalMap;
		GLint materialUseDiffuseMap;
		GLint materialUseSpecularMap;
		GLint useTexture;
		GLint texSampler;

		GLint materialUseMaps;

		GLint lightBlockIdx;
		UniformBufferObject* lightUBO;
	} uniforms;

	LightingShader();
	LightingShader(std::string vertexFile, std::string fragmentFile);

	~LightingShader();

	/// Set material uniforms
	void setMaterial(Material* material) const override;
	/// Set transform uniforms
	void setTransformParameters(const Camera& camera, const glm::mat4& model) const override;

	/// Add light to the UBO
	void addLight(const Light* light, const glm::vec3& position, const glm::vec3& direction);
	/// Reset light UBO
	void resetLights();
};

#endif