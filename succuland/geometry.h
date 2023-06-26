#pragma once

#ifndef _GEOMETRY_H
#define _GEOMETRY_H 

#include "pgr.h"
#include "shader.h"
#include "properties.h"
#include "perlin.h"

#include <algorithm>
#include <iostream>
#include <glm/ext.hpp>

///	Macro for defining what parameters does the geometry use
#define COLOR_BIT			0b0001
#define NORMAL_BIT			0b0010
#define TEXTURE_BIT			0b0100

/// Class defining generic mesh
class Mesh 
{
protected:
	GLuint vbo;
	GLuint ebo;
	GLuint vao;

	/// Flags indicating what information does the mesh contain
	uint8_t flags;

	/// Offset of normals in VBO
	long normalOffset;
	/// Offset of colors in VBO
	long colorOffset;
	/// Offset of texture coordinates in VBO
	long texOffset;
	/// Size of vertex set
	long vertexSetSize;

	/// Initialize offsets using flags
	virtual void initOffsets();
	/// Initialize buffers using flags and offsets
	virtual void initBuffers();
	virtual void setPositionData(void* data);
	virtual void setNormalData(void* data);
	virtual void setColorData(void* data);
	virtual void setTexData(void* data);

	/// Initialize mesh parameters without creating buffers, used by derived classes
	Mesh(Shader* shader, uint8_t flags, int numVertices, int numPrimitives, long setSize); 

public:
	/// Number of primitives mesh contains
	unsigned int numPrimitives;
	///	Number of vertices mesh contains, doesn't count duplicates
	unsigned int numVertices;
	Shader* shader;
	
	Mesh();
	Mesh(float* data, unsigned int* indices, unsigned int numPrimitives, unsigned int numVertices, Shader* shader, uint8_t flags);
	~Mesh();

	/// Low level draw call to render current mesh, doesn't set any parameters, doesn't bind any shaders
	virtual void draw() const;
};

/// Mesh with materials
class TexturedMesh : public Mesh 
{
protected:
	Material* material;

	TexturedMesh(Shader* shader, Material* material, uint8_t flags, int numVertices, int numPrimitives, long setSize);

public:
	TexturedMesh();
	TexturedMesh(float* data, unsigned int* indices, unsigned int numPrimitives, unsigned int numVertices, Shader* shader, Material* material, uint8_t flags);

	/// Low level draw call to render current mesh, additionally sets material uniforms
	void draw() const override;
};

/// Mesh for terrain generated using perling noise
class TerrainMesh : public TexturedMesh 
{
protected:
	const unsigned int width;
	const unsigned int height;

	/// Perling noise defining height of the terrain
	const Perlin perlin;

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> colors;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texCoords;

	std::vector<unsigned int> indices;

	/// Generate terrain mesh data using constructor parameters
	void generate();

public:
	TerrainMesh();
	TerrainMesh(unsigned int width, unsigned int height, float scale, int seed, Shader* shader, Material* material, uint8_t flags);

	/// Low level draw call to render current mesh, additionally sets material uniforms (uses triangle strips)
	void draw() const override;
	/// Gets a reference to meshes perlin noise function
	const Perlin& getPerlin() const;
};

/// Mesh that can be loaded from a file, handles multiple sub meshes and materials
class OBJMesh : public TexturedMesh 
{
protected:

	/// List of references to sub meshes
	std::vector<Mesh*> subMeshes;

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> colors;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texCoords;
	std::vector<unsigned int> indices;

	/// Load mesh data from file to internal data structures
	void loadFile(const std::string& path);
	/// Load mesh data from a Assimp Mesh and Assimp Material objects
	void loadMesh(const aiMesh* mesh, const aiMaterial* aiMaterial, const std::string& path);


	OBJMesh(const aiMesh* mesh, const aiMaterial* mat, const std::string& path, Shader* shader);

public:
	OBJMesh();
	OBJMesh(const std::string& path, Shader* shader);
	~OBJMesh();

	/// Low level draw call, draws current mesh and all sub meshes, additionally sets material uniforms
	void draw() const override;
};

#endif