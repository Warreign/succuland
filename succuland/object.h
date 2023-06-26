#pragma once

#ifndef _OBJECT_H
#define _OBJECT_H

#include "pgr.h"
#include "shader.h"
#include "geometry.h"
#include "properties.h"
#include <iostream>
#include <filesystem>

class Camera;

/// Generic drawable object
class ObjectInstance 
{
protected:
	/// Pointer to mesh the object uses
	Mesh* geometry;

	/// Model matrix of the object
	glm::mat4 model;

	/// Children objects 
	std::vector<ObjectInstance*> children;

public:
	ObjectInstance(Mesh* geometry, glm::mat4 model);

	/// Position of the object
	glm::vec3 position;

	/// Add child to vector of children
	void addChild(ObjectInstance* newChild);

	///	<summary>
	///  Draw current object using it's geometry and transform parameters, draw children objects
	/// </summary>
	/// <param name="camera">Camera object for view and projection matrices</param>
	virtual void draw(const Camera& camera) const;

	/// Update model matrix
	virtual void update(const glm::mat4& model);
	/// Rotate the object in it's model space
	virtual void rotate(float degrees, const glm::vec3& axis);
	/// Move object to new position maintaining other transformations
	virtual void move(const glm::vec3& new_position);
	/// Move object to new position, change direction (doesn't change the rotation, mainly used by derived classes)
	virtual void move(const glm::vec3& new_position, const glm::vec3& new_direction);
};

/// Object additionally storing and setting light parameters
class LightObject : public ObjectInstance
{
protected:
	/// Pointer to shader object, that handles lighting
	LightingShader* lshader;
	/// Pointer to light properties
	Light* light;
	
	/// Direction of light (used by directional and spotlight),
	glm::vec3 direction;

public:

	/// Constructor for a directional light
	LightObject(Light* light, LightingShader* lightingShader, const glm::vec3 direction);
	LightObject(Mesh* geometry, Light* light, LightingShader* lightingShader, const glm::mat4& model);
	LightObject(Mesh* geometry, Light* light, LightingShader* lightingShader, const glm::mat4& model, const glm::vec3& direction);
	LightObject(Mesh* geometry, Light* light, LightingShader* lightingShader, const glm::vec3& position);
	LightObject(Mesh* geomtery, Light* light, LightingShader* lightingShader, const glm::vec3& position, const glm::vec3& direction);

	///	<summary>
	///  Draw current light object if it has geometry, additionaly sets light parameters
	/// </summary>
	/// <param name="camera">Camera object for view and projection matrices</param>
	void draw(const Camera& camera) const override;

	/// Update model matrix
	void update(const glm::mat4& model);
	/// Move object to new position maintaining other transformations
	void move(const glm::vec3& new_position);
	/// Move object to new position, change direction
	void move(const glm::vec3& new_position, const glm::vec3& new_direction);
};

/// Skybox object
class Skybox : ObjectInstance 
{
protected:
	/// Cube map texture handle
	GLuint texture;
public:
	/// <summary>
	/// Load cube map from a folder (only supports 6 separate images)
	/// </summary>
	/// <param name="path">path to folder containing 6 images named px, nx, py, ny, pz, nz</param>
	/// <returns>handle for cube map</returns>
	static GLuint loadTexture(const std::string& path);


	Skybox(Mesh* geometry, const std::string& folderPath);

	///	<summary>
	///  Draw the skybox
	/// </summary>
	/// <param name="camera">Camera object for view and projection matrices</param>
	void draw(const Camera& camera) const override; 
};

class Banner : ObjectInstance 
{
protected:
	/// Handle for a banner texture
	GLuint texture;

public:

	/// Create banner from a file
	Banner(Mesh* geometry, const std::string& path);

	///  Draw banner without depth test, using blending
	void draw(const Camera& camera) const override;
};

/// Arrow with spinning animation
class Arrow : public ObjectInstance
{
protected:
	/// Starting rotation and scale of an arrow
	glm::mat4 initialModel;

public:
	/// Elevation of the arrow during spinning animation
	float elevation;
	/// Radius of spinning animation
	float radius;
	/// Point the arrow is targeted while spinning
	glm::vec3 target;
	/// Index of the object the arrow is spinning above
	uint8_t currentIdx;

	Arrow(Mesh* geometry, float elevation, float radius, const glm::mat4& model);

	/// Step of a arrow spinning animation 
	void animationStep();
};

/// Particle sprite with changing texture animation
class Particle : public ObjectInstance
{
protected:
	/// Vector of all particles that are currently being rendered
	static std::vector<Particle*> particles;
	/// Geometry of a particle (mainly containing a quad)
	static Mesh* partGeometry;

	/// Time in milliseconds over which the particle is animated
	uint32_t animationTime;
	/// Handle for texture containing animation images
	GLuint texture;
	/// Animation start time
	uint32_t startTime;
	/// Current frame of the animation
	uint32_t frame;

public:
	/// Construct an animated particle sprite
	Particle(const std::string& path, uint32_t animationTime, const glm::vec3& position);

	/// Draw the particle using blending
	void draw(const Camera& camera) const override;
	/// Destroy current particle and remove it from the particle list
	void destroy() const;

	/// Initialize geometry
	static void init(Mesh* geometry);
	/// Create particle object at given position
	static void createParticle(const std::string& texturePath, const glm::vec3& position);
	/// Get a reference to the static list of particles
	static const std::vector<Particle*>& getParticles();
};

#endif