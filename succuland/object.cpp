#include "object.h"

/*
 *	Generic object
 */

ObjectInstance::ObjectInstance(Mesh* geometry, glm::mat4 model) :
	geometry(geometry), model(model), position(model[3])
{
}

void ObjectInstance::addChild(ObjectInstance* newChild)
{
	newChild->update(this->model);
	children.push_back(newChild);
}

void ObjectInstance::draw(const Camera& camera) const
{
	for (const auto& child : children)
	{
		child->draw(camera);
	}
	geometry->shader->use();
	geometry->shader->setTransformParameters(camera, model);
	geometry->shader->loadFog();
	geometry->draw();
	Shader::unbind();

}

void ObjectInstance::update(const glm::mat4& updateModel) 
{
	model = updateModel * model;
	position = model[3];
}

void ObjectInstance::rotate(float degrees, const glm::vec3& axis)
{
	glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), glm::radians(degrees), axis);
	glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(this->model[3]));
	this->model = glm::mat4(glm::mat3(this->model));
	this->model = translate * rotate * this->model;

	for (const auto& child : children)
	{
		child->rotate(degrees, axis);
	}
}

void ObjectInstance::move(const glm::vec3& new_position) 
{
	model = glm::translate(glm::mat4(1.0f), new_position) * glm::mat4(glm::mat3(model));
	position = new_position;
}

void ObjectInstance::move(const glm::vec3& new_position, const glm::vec3& new_direction) 
{
	move(new_position);
}

/*
*	Light object
*/

LightObject::LightObject(Light* light, LightingShader* lightingShader, const glm::vec3 direction)
	: ObjectInstance(nullptr, glm::mat4(1.0f)),
	light(light), lshader(lightingShader), direction(direction)
{
}

LightObject::LightObject(Mesh* geometry, Light* light, LightingShader* lightingShader, const glm::mat4& model)
	: ObjectInstance(geometry, model),
	light(light), lshader(lightingShader), direction(glm::vec3(0.0f))
{
}

LightObject::LightObject(Mesh* geometry, Light* light, LightingShader* lightingShader, const glm::mat4& model, const glm::vec3& direction)
	: ObjectInstance(geometry, model),
	light(light), lshader(lightingShader), direction(direction)
{
}

LightObject::LightObject(Mesh* geometry, Light* light, LightingShader* lightingShader, const glm::vec3& position)
	: ObjectInstance(geometry, glm::translate(glm::mat4(1.0f), position)),
	light(light), lshader(lightingShader)
{
}

LightObject::LightObject(Mesh* geometry, Light* light, LightingShader* lightingShader, const glm::vec3& position, const glm::vec3& direction)
	: ObjectInstance(geometry, glm::translate(glm::mat4(1.0f), position)),
	light(light), lshader(lightingShader), direction(direction)
{
}

void LightObject::draw(const Camera& camera) const 
{
	lshader->addLight(light, position, direction);

	if (light->type != LIGHT_DIRECTIONAL && light->type != LIGHT_SPOTLIGHT) {
		geometry->shader->use();
		geometry->shader->setTransformParameters(camera, model);
		geometry->shader->loadFog();
		geometry->draw();
		Shader::unbind();
	}
}

void LightObject::update(const glm::mat4& model) 
{
	this->model = model * this->model;
	position = this->model[3];
}

void LightObject::move(const glm::vec3& new_position)
{
	position = new_position;
	if (light->type == LIGHT_POINT || light->type == LIGHT_SPOTLIGHT) {
		model = glm::translate(glm::mat4(1.0f), new_position) * glm::mat4(glm::mat3(model));
	}
}

void LightObject::move(const glm::vec3& new_position, const glm::vec3& new_direction)
{
	direction = new_direction;
	move(new_position);
}

/*
 *	Skybox
 */

Skybox::Skybox(Mesh* geometry, const std::string& folderPath) : 
	ObjectInstance(geometry, glm::mat4(1.0f)),
	texture(loadTexture(folderPath)) {}

GLuint Skybox::loadTexture(const std::string& path) {
	GLuint skyboxTexture;
	glGenTextures(1, &skyboxTexture);
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);

	std::filesystem::directory_entry file = *std::filesystem::directory_iterator(path);
	std::string filename = file.path().string();
	std::string ext = filename.substr(filename.find_last_of(".") + 1);
	std::vector<std::string> faces = {
		"px",
		"nx",
		"py",
		"ny",
		"pz",
		"nz"
	};

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	for (unsigned int i = 0; i < faces.size(); ++i) {
		if (!pgr::loadTexImage2D(path + "/" + faces[i] + "." + ext, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i)) {
			throw std::runtime_error("could not load skybox file: " + faces[i] + "." + ext);
		}
	}

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glActiveTexture(GL_TEXTURE0);

	return skyboxTexture;
}

void Skybox::draw(const Camera& camera) const 
{
	glDepthFunc(GL_LEQUAL);

	geometry->shader->use();
	geometry->shader->setTransformParameters(camera, model);
	geometry->shader->loadFog();

	geometry->shader->setInteger("skybox", 10);
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	geometry->draw();
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glActiveTexture(GL_TEXTURE0);

	Shader::unbind();
	glDepthFunc(GL_LESS);
}

/*
*	Banner
*/

Banner::Banner(Mesh* geometry, const std::string& path)
	: ObjectInstance(geometry, glm::mat4(1.0f)),
	texture(pgr::createTexture(path))
{
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
}

void Banner::draw(const Camera& camera) const
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	geometry->shader->use();
	geometry->shader->setFloat("time", float(glutGet(GLUT_ELAPSED_TIME)));

	geometry->shader->setInteger("banner", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	geometry->draw();
	glBindTexture(GL_TEXTURE_2D, 0);

	CHECK_GL_ERROR();

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	Shader::unbind();
}

/*
*	Arrow
*/

Arrow::Arrow(Mesh* geometry, float elevation, float radius, const glm::mat4& model)
	: ObjectInstance(geometry, model),
	initialModel(model), currentIdx(0), elevation(elevation), radius(radius)
{
}

void Arrow::animationStep()
{
	float alpha = glutGet(GLUT_ELAPSED_TIME) / 100.0f;
	glm::mat4 newModel = glm::inverse(glm::lookAt(target + glm::vec3(glm::sin(alpha) * radius, elevation, glm::cos(alpha) * radius), target, glm::vec3(0, 1, 0)));
	model = newModel * initialModel;
}

/*
*	Particle
*/

Mesh* Particle::partGeometry = nullptr;
std::vector<Particle*> Particle::particles;

Particle::Particle(const std::string& path, uint32_t animationTime, const glm::vec3& position)
	: ObjectInstance(partGeometry, glm::translate(glm::mat4(1.0f), position + glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(10.0f))),
	texture(pgr::createTexture(path)), animationTime(animationTime), startTime(glutGet(GLUT_ELAPSED_TIME))
{
}

void Particle::draw(const Camera& camera) const
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	geometry->shader->use();
	geometry->shader->setTransformParameters(camera, model);
	geometry->shader->setFloat("time", glutGet(GLUT_ELAPSED_TIME));

	uint32_t frames = 14;
	uint32_t time = glutGet(GLUT_ELAPSED_TIME) - startTime;
	uint32_t frame = (time * frames) / animationTime;
	geometry->shader->setInteger("frame", frame);

	geometry->shader->setInteger("particle", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	geometry->draw();
	glBindTexture(GL_TEXTURE_2D, 0);

	Shader::unbind();
	glDisable(GL_BLEND);

	if (frame >= frames-1)
	{
		destroy();
	}
}

void Particle::destroy() const 
{
	particles.erase(std::remove(particles.begin(), particles.end(), this));
	delete this;
}

void Particle::init(Mesh* geometry)
{
	partGeometry = geometry;
}

void Particle::createParticle(const std::string& texturePath, const glm::vec3& position)
{
	if (!partGeometry)
		throw std::runtime_error("Particle geometry not initialized");

	particles.push_back(new Particle(texturePath, 1000, position));
}

const std::vector<Particle*>& Particle::getParticles()
{
	return particles;
}
