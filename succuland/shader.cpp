#include "shader.h"
#include "properties.h"


/*
*	Base shader
*/

Fog* Shader::fog = nullptr;

Shader::Shader() 
	: program(0),attributes({ -1 }), uniforms({ -1 })
{
}

Shader::Shader(std::string vertFileName, std::string fragFileName) : Shader() 
{
	
	GLuint vertexShader = pgr::createShaderFromFile(GL_VERTEX_SHADER, vertFileName);
	if (vertexShader == 0) {
		throw std::runtime_error("Failed to compile vertex shader");
	}

	GLuint fragmentShader = pgr::createShaderFromFile(GL_FRAGMENT_SHADER, fragFileName);
	if (fragmentShader == 0) {
		throw std::runtime_error("Failed to compile fragment shader");
	}

	GLuint shaders[] = { vertexShader, fragmentShader, 0 };
	GLuint program = pgr::createProgram(shaders);
	if (program == 0) {
		throw std::runtime_error("Failed to compile program");
	}

	this->program = program;
	attributes.position = glGetAttribLocation(program, "aPosition");
	attributes.color = glGetAttribLocation(program, "aColor");
	attributes.normal = glGetAttribLocation(program, "aNormal");
	attributes.texCoord = glGetAttribLocation(program, "aTexCoord");

	uniforms.PVM = glGetUniformLocation(program, "PVM");
	uniforms.ViewM = glGetUniformLocation(program, "ViewM");
	uniforms.ModelM = glGetUniformLocation(program, "ModelM");
	uniforms.ProjectM = glGetUniformLocation(program, "ProjectM");
}

void Shader::unbind() 
{
	glUseProgram(0);
}

void Shader::setFog(Fog* fog)
{
	Shader::fog = fog;
}

void Shader::use() const 
{
	glUseProgram(program);
}

void Shader::clear() {
	pgr::deleteProgramAndShaders(program);
}

void Shader::setMaterial(Material* material) const 
{
}

void Shader::loadFog() const
{
	setVec3("fog.color", fog->color);
	setFloat("fog.density", fog->currentDensity);
	setFloat("fog.gradient", fog->gradient);
	setInteger("fog.isEnabled", fog->isVisible);
}

void Shader::setTransformParameters(const Camera& camera, const glm::mat4& model) const 
{
	glUniformMatrix4fv(uniforms.PVM, 1, GL_FALSE, glm::value_ptr(camera.projectMatrix() * camera.viewMatrix() * model));
	glUniformMatrix4fv(uniforms.ViewM, 1, GL_FALSE, glm::value_ptr(camera.viewMatrix()));
	glUniformMatrix4fv(uniforms.ModelM, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(uniforms.ProjectM, 1, GL_FALSE, glm::value_ptr(camera.projectMatrix()));
}

void Shader::setInteger(const std::string uniformName, int value) const 
{
	GLint location = glGetUniformLocation(program, uniformName.c_str());
	glUniform1i(location, value);
}

void Shader::setFloat(const std::string uniformName, float value) const 
{
	GLint location = glGetUniformLocation(program, uniformName.c_str());
	glUniform1f(location, value);
}

void Shader::setVec2(const std::string uniformName, glm::vec2& value) const 
{
	GLint location = glGetUniformLocation(program, uniformName.c_str());
	glUniform2fv(location, 1, glm::value_ptr(value));
}

void Shader::setVec3(const std::string uniformName, glm::vec3& value) const 
{
	GLint location = glGetUniformLocation(program, uniformName.c_str());
	glUniform3fv(location, 1, glm::value_ptr(value));
}

void Shader::setMat4(const std::string uniformName, glm::mat4& value) const 
{
	GLint location = glGetUniformLocation(program, uniformName.c_str());
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}


/*
*	Lighting shader
*/

LightingShader::LightingShader() 
	: Shader(), 
	uniforms({ -1 }), lightsLoadedNum(0) 
{
}

LightingShader::LightingShader(std::string vertexFile, std::string fragmentFile) 
	: Shader(vertexFile, fragmentFile),
	lightsLoadedNum(0)
{
	uniforms.PVM = glGetUniformLocation(program, "PVM");
	uniforms.ViewM = glGetUniformLocation(program, "ViewM");
	uniforms.ModelM = glGetUniformLocation(program, "ModelM");
	uniforms.NormalM = glGetUniformLocation(program, "NormalM");
	uniforms.ProjectM = glGetUniformLocation(program, "ProjectM");
	uniforms.cameraPos = glGetUniformLocation(program, "cameraPos");

	uniforms.materialAmbient = glGetUniformLocation(program, "material.ambient");
	uniforms.materialDiffuse = glGetUniformLocation(program, "material.diffuse");
	uniforms.materialSpecular = glGetUniformLocation(program, "material.specular");
	uniforms.materialShininess = glGetUniformLocation(program, "material.shininess");

	uniforms.useTexture = glGetUniformLocation(program, "material.useTexture");
	uniforms.texSampler = glGetUniformLocation(program, "sampler");

	uniforms.materialDiffuseMap = glGetUniformLocation(program, "material.diffuseMap");
	uniforms.materialSpecularMap = glGetUniformLocation(program, "material.specularMap");
	uniforms.materialNormalMap = glGetUniformLocation(program, "material.normalMap");
	uniforms.materialUseDiffuseMap = glGetUniformLocation(program, "material.useDiffuseMap");
	uniforms.materialUseSpecularMap = glGetUniformLocation(program, "material.useSpecularMap");

	uniforms.materialUseMaps = glGetUniformLocation(program, "material.useMaps");

	uniforms.lightBlockIdx = glGetUniformBlockIndex(program, "Lights");

	GLint lightBlockSize = 0;
	glGetActiveUniformBlockiv(program, uniforms.lightBlockIdx, GL_UNIFORM_BLOCK_DATA_SIZE, &lightBlockSize);

	uniforms.lightUBO = new UniformBufferObject(lightBlockSize, LIGHTS_BINDING_POINT);
	glUniformBlockBinding(program, uniforms.lightBlockIdx, LIGHTS_BINDING_POINT);

}

LightingShader::~LightingShader()
{
	delete uniforms.lightUBO;
}

void LightingShader::setTransformParameters(const Camera& camera, const glm::mat4& model) const 
{
	glUniformMatrix4fv(uniforms.PVM, 1, GL_FALSE, glm::value_ptr(camera.projectMatrix() * camera.viewMatrix() * model));
	glUniformMatrix4fv(uniforms.ViewM, 1, GL_FALSE, glm::value_ptr(camera.viewMatrix()));
	glUniformMatrix4fv(uniforms.ModelM, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(uniforms.ProjectM, 1, GL_FALSE, glm::value_ptr(camera.projectMatrix()));
	glUniformMatrix4fv(uniforms.NormalM, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(model))));
	glUniform3fv(uniforms.cameraPos, 1, glm::value_ptr(camera.position));
}

void LightingShader::setMaterial(Material* material) const
{
	MaterialMap* mat = dynamic_cast<MaterialMap*>(material);

	glUniform3fv(uniforms.materialAmbient, 1, glm::value_ptr(material->ambient));
	glUniform3fv(uniforms.materialDiffuse, 1, glm::value_ptr(material->diffuse));
	glUniform3fv(uniforms.materialSpecular, 1, glm::value_ptr(material->specular));
	glUniform1f(uniforms.materialShininess, material->shininess);

	if (mat) 
	{
		int tex = 0;

		if (mat->diffuseMap) 
		{
			glUniform1i(uniforms.materialUseDiffuseMap, 1);
			glUniform1i(uniforms.materialDiffuseMap, tex);
			glActiveTexture(GL_TEXTURE0 + tex++);
			glBindTexture(GL_TEXTURE_2D, mat->diffuseMap);
		}
		
		if (mat->specularMap) 
		{
			glUniform1i(uniforms.materialUseSpecularMap, 1);
			glUniform1i(uniforms.materialSpecularMap, tex);
			glActiveTexture(GL_TEXTURE0 + tex++);
			glBindTexture(GL_TEXTURE_2D, mat->specularMap);
		}
	}
	else 
	{
		glUniform1i(uniforms.materialUseDiffuseMap, 0);
		glUniform1i(uniforms.materialUseSpecularMap, 0);
	}
}

void LightingShader::addLight(const Light* light, const glm::vec3& position, const glm::vec3& direction)
{
	if (lightsLoadedNum >= MAX_LIGHT_NUM) 
	{
		throw std::runtime_error("Exceeded maximum number of lights (" + std::to_string(MAX_LIGHT_NUM) + ")");
	}
	++lightsLoadedNum;
	uniforms.lightUBO->setData(0, &lightsLoadedNum, 16);
	GLSLLight* data = light->toDataPtr();
	data->position = position;
	data->direction = direction;
	uniforms.lightUBO->setData(16 + (lightsLoadedNum-1) * sizeof(GLSLLight), data, sizeof(GLSLLight));
	delete data;
}

void LightingShader::resetLights()
{
	lightsLoadedNum = 0;
	uniforms.lightUBO->setData(0, &lightsLoadedNum, 16);
}

/*
*	UBO wrapper
*/

UniformBufferObject::UniformBufferObject()
	: bufferID(0)
{
}

UniformBufferObject::~UniformBufferObject()
{
	glDeleteBuffers(1, &bufferID);
}

UniformBufferObject::UniformBufferObject(const size_t byteSize, GLuint bindingPoint, GLenum usageHint)
	: byteSize(byteSize)
{
	glGenBuffers(1, &bufferID);
	glBindBuffer(GL_UNIFORM_BUFFER, bufferID);
	glBufferData(GL_UNIFORM_BUFFER, byteSize, nullptr, usageHint);
	glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, bufferID);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBufferObject::setData(const size_t offset, const void* data, const size_t dataSize) {
	glBindBuffer(GL_UNIFORM_BUFFER, bufferID);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, dataSize, data);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}








