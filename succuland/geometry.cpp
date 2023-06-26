#include "geometry.h"

/*
*	Geometry
*/

Mesh::Mesh() 
	: vbo(0), ebo(0), vao(0),
	flags(0), numPrimitives(0), numVertices(0), shader(nullptr) 
{
}

Mesh::Mesh(Shader* shader, uint8_t flags, int numVertices, int numPrimitives, long setSize)
	: shader(shader), flags(flags), numVertices(numVertices), numPrimitives(numPrimitives), vertexSetSize(setSize)
{
}

Mesh::Mesh(float* data, unsigned int* indices, unsigned int numPrimitives, unsigned int numVertices, Shader* shader, uint8_t flags)
	: shader(shader), numPrimitives(numPrimitives), numVertices(numVertices), flags(flags), vertexSetSize(numPrimitives * 3 * sizeof(float))
{
	initOffsets();
	initBuffers();

	setPositionData(data);
	if (flags & NORMAL_BIT)
		setNormalData((char*)data + normalOffset);
	if (flags & COLOR_BIT)
		setColorData((char*)data + colorOffset);
	if (flags & TEXTURE_BIT)
		setTexData((char*)data + texOffset);
}

void Mesh::initOffsets() 
{
	bool normal = flags & NORMAL_BIT;
	bool color = flags & COLOR_BIT;
	bool tex = flags & TEXTURE_BIT;

	colorOffset = 3 * vertexSetSize;
	normalOffset = (normal + color) * 3 * vertexSetSize;
	texOffset = (normal + color + tex) * 3 * vertexSetSize;
}

void Mesh::initBuffers() 
{
	bool normal = flags & NORMAL_BIT;
	bool color = flags & COLOR_BIT;
	bool tex = flags & TEXTURE_BIT;

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	glEnableVertexAttribArray(shader->attributes.position);
	glVertexAttribPointer(shader->attributes.position, 3, GL_FLOAT, GL_FALSE, 0, 0);

	if (normal) 
	{
		glEnableVertexAttribArray(shader->attributes.normal);
		glVertexAttribPointer(shader->attributes.normal, 3, GL_FLOAT, GL_FALSE, 0, (void*)normalOffset);
	}

	if (color) 
	{
		glEnableVertexAttribArray(shader->attributes.color);
		glVertexAttribPointer(shader->attributes.color, 3, GL_FLOAT, GL_FALSE, 0, (void*)colorOffset);
	}

	if (tex) 
	{
		glEnableVertexAttribArray(shader->attributes.texCoord);
		glVertexAttribPointer(shader->attributes.texCoord, 2, GL_FLOAT, GL_FALSE, 0, (void*)texOffset);
	}

	glBufferData(GL_ARRAY_BUFFER, vertexSetSize * (3 + 3 * normal + 3 * color + 2 * tex), nullptr, GL_STATIC_DRAW);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::setPositionData(void* data) 
{
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * vertexSetSize, data);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::setNormalData(void* data) 
{
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferSubData(GL_ARRAY_BUFFER, normalOffset, 3 * vertexSetSize, data);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::setColorData(void* data) 
{
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferSubData(GL_ARRAY_BUFFER, colorOffset, 3 * vertexSetSize, data);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::setTexData(void* data) 
{
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferSubData(GL_ARRAY_BUFFER, texOffset, 2 * vertexSetSize, data);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Mesh::~Mesh() 
{
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
	glDeleteVertexArrays(1, &vao);
}

void Mesh::draw() const 
{
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 3 * numPrimitives);
	glBindVertexArray(0);
}

/*
*	Textured Mesh
*/

TexturedMesh::TexturedMesh(Shader* shader, Material* material, uint8_t flags, int numVertices, int numPrimitives, long setSize)
	: Mesh(shader, flags, numVertices, numPrimitives, setSize),
	material(material)
{
}

TexturedMesh::TexturedMesh()
	: Mesh(),
	material(nullptr)
{
}

TexturedMesh::TexturedMesh(float* data, unsigned int* indices, unsigned int numPrimitives, unsigned int numVertices, Shader* shader, Material* material, uint8_t flags)
	: Mesh(data, indices, numPrimitives, numVertices, shader, flags),
	material(material)
{
}

void TexturedMesh::draw() const
{
	shader->setMaterial(material);
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 3 * numPrimitives);
	glBindVertexArray(0);
}


/*
*	Terrain mesh
*/

TerrainMesh::TerrainMesh()
	: TexturedMesh(), 
	width(0), height(0) 
{
}

TerrainMesh::TerrainMesh(unsigned int width, unsigned int height, float scale, int seed, Shader* shader, Material* material, uint8_t flags)
	: TexturedMesh(shader, material, flags, width* height, height -1, width* height * sizeof(float)), 
	width(width), height(height), perlin(Perlin(2, 0.012f, 20.0f, seed))
{
 	generate();
	initOffsets();
	initBuffers();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	setPositionData(vertices.data());
	if (flags & NORMAL_BIT)
		setNormalData(normals.data());
	if (flags & COLOR_BIT)
		setColorData(colors.data());
	if (flags & TEXTURE_BIT)
		setTexData(texCoords.data());
}

void TerrainMesh::generate() 
{

	// Fill vertices
	vertices.reserve(numVertices);
	for (unsigned int i = 0; i < height; ++i)
	{
		for (unsigned int j = 0; j < width; ++j) 
		{
			float x = -(width / 2.0f) + j;
			float z = -(height / 2.0f) + i;
			vertices.emplace_back(x, perlin(x,z), z);
		}
	}

	// Fill indices
	indices.reserve(width * 2 * (height - 1));
	for (unsigned int i = 0; i < height - 1; ++i) 
	{
		for (unsigned int j = 0; j < width; ++j) 
		{
			indices.push_back(i * width + j);
			indices.push_back((i + 1)*width + j);
		}
	}

	// Fill normals
	if (flags & NORMAL_BIT) 
	{
		float off = 0.2;
		normals.reserve(numVertices);
		for (int i = 0; i < height; ++i) 
		{
			for (int j = 0; j < width; ++j) 
			{
				const glm::vec3& vertex = vertices[i * height + j];
				volatile float x = vertex.x + off;
				volatile float z = vertex.z;
				glm::vec3 up = glm::vec3(x, perlin(x,z), z);
				x = vertex.x;
				z = vertex.z + off;
				glm::vec3 right = glm::vec3(x, perlin(x, z), z);
				x = vertex.x - off;
				z = vertex.z;
				glm::vec3 down = glm::vec3(x, perlin(x, z), z);
				x = vertex.x;
				z = vertex.z - off;
				glm::vec3 left = glm::vec3(x, perlin(x, z), z);
				normals.push_back(glm::normalize(glm::cross(right - left, up - down)));
			}
		}
	}

	if (flags & TEXTURE_BIT) 
	{
		float spacing = 0.05;
		float u = 0.0f;
		float v = 0.0f;
		texCoords.reserve(width * height);
		for (int i = 0; i < height; ++i) 
		{
			for (int j = 0; j < width; ++j) 
			{
				texCoords.emplace_back(u, v);
				u += spacing;
			}
			v += spacing;
			u = 0.0f;
		}
	}
}

void TerrainMesh::draw() const 
{
	shader->setMaterial(material);
	glBindVertexArray(vao);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	for (unsigned int s = 0; s < height - 1; s++) 
	{
		glDrawElements(GL_TRIANGLE_STRIP, width * 2, GL_UNSIGNED_INT, (void*)(width * 2 * s * sizeof(unsigned int)));
	}
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindVertexArray(0);
}

const Perlin& TerrainMesh::getPerlin() const 
{
	return perlin;
}

/*
*	OBJ Mesh
*/

OBJMesh::OBJMesh()
	: TexturedMesh()
{
}

OBJMesh::OBJMesh(const std::string& path, Shader* shader)
	: TexturedMesh(shader, nullptr, 0, 0, 0, 0)
{
	loadFile(path);
	initOffsets();
	initBuffers();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	setPositionData(vertices.data());
	if (flags & NORMAL_BIT)
		setNormalData(normals.data());
	if (flags & COLOR_BIT)
		setColorData(colors.data());
	if (flags & TEXTURE_BIT)
		setTexData(texCoords.data());
}

OBJMesh::~OBJMesh()
{
	delete material;
	for (const auto& m : subMeshes)
	{
		delete m;
	}
}

OBJMesh::OBJMesh(const aiMesh* mesh, const aiMaterial* mat, const std::string& path, Shader* shader)
	: TexturedMesh(shader, nullptr, 0, 0, 0, 0)
{
	loadMesh(mesh, mat, path);
	initOffsets();
	initBuffers();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	setPositionData(vertices.data());
	if (flags & NORMAL_BIT)
		setNormalData(normals.data());
	if (flags & COLOR_BIT)
		setColorData(colors.data());
	if (flags & TEXTURE_BIT)
		setTexData(texCoords.data());
}

void OBJMesh::loadFile(const std::string& path)
{
	std::cout << "INFO: loading " << path << std::endl;

	Assimp::Importer importer;

	importer.SetPropertyInteger(AI_CONFIG_PP_PTV_NORMALIZE, 1);

	const aiScene* scn = importer.ReadFile(path.c_str(),
		aiProcess_Triangulate |
		//aiProcess_PreTransformVertices |
		aiProcess_GenSmoothNormals |
		aiProcess_JoinIdenticalVertices |
		aiProcess_TransformUVCoords |
		0);


	if (scn == nullptr) 
	{
		throw std::runtime_error(importer.GetErrorString());
	}

	if (scn->mNumMeshes < 1) 
	{
		throw std::runtime_error("no meshes found in scene " + path);
	}

	std::cout << "INFO: loaded " << scn->mNumMeshes << " meshes" << std::endl;

	auto m = scn->mMeshes[0];
	const aiMaterial* mat = scn->mMaterials[m->mMaterialIndex];
	loadMesh(m, mat, path);
	
	for (int i = 1; i < scn->mNumMeshes; ++i) 
	{
		m = scn->mMeshes[i];
		mat = scn->mMaterials[m->mMaterialIndex];

		subMeshes.push_back(new OBJMesh(m, mat, path, shader));
	}
}

void OBJMesh::loadMesh(const aiMesh* m, const aiMaterial* mat, const std::string& path)
{
	const uint32_t FACE_VERT_COUNT = 3;

	vertices.reserve(m->mNumVertices);
	for (int i = 0; i < m->mNumVertices; ++i) 
	{
		auto v = m->mVertices[i];
		vertices.emplace_back(v.x, v.y, v.z);
	}

	if (m->mNormals != nullptr) 
	{
		flags |= NORMAL_BIT;
		for (int i = 0; i < m->mNumVertices; ++i) 
		{
			auto vn = m->mNormals[i];
			normals.emplace_back(vn.x, vn.y, vn.z);
		}
	}

	if (m->mTextureCoords[0] != nullptr) 
	{
		flags |= TEXTURE_BIT;
		for (int i = 0; i < m->mNumVertices; ++i) 
		{
			auto vt = m->mTextureCoords[0][i];
			texCoords.emplace_back(vt.x, vt.y);
		}
	}

	indices.resize(m->mNumFaces * FACE_VERT_COUNT);
	for (int i = 0; i < m->mNumFaces; ++i) 
	{
		auto f = m->mFaces[i];
		memcpy(indices.data() + i * FACE_VERT_COUNT, m->mFaces[i].mIndices, FACE_VERT_COUNT * sizeof(unsigned int));
	}

	aiColor4D color;

	glm::vec3 ambient(0.0f);
	glm::vec3 diffuse(0.0f);
	glm::vec3 specular(0.0f);

	if (aiGetMaterialColor(mat, AI_MATKEY_COLOR_AMBIENT, &color) == AI_SUCCESS)
	{
		ambient.r = color.r;
		ambient.g = color.g;
		ambient.b = color.b;
	}
	if (aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &color) == AI_SUCCESS)
	{
		diffuse.r = color.r;
		diffuse.g = color.g;
		diffuse.b = color.b;
	}
	if (aiGetMaterialColor(mat, AI_MATKEY_COLOR_SPECULAR, &color) == AI_SUCCESS)
	{
		specular.r = color.r;
		specular.g = color.g;
		specular.b = color.b;
	}

	ai_real shine = 1.0f, strength = 1.0f;
	uint32_t max;
	max = 1;
	if (aiGetMaterialFloatArray(mat, AI_MATKEY_SHININESS, &shine, &max) != AI_SUCCESS)
		shine = 1.0f;
	max = 1;
	if (aiGetMaterialFloatArray(mat, AI_MATKEY_SHININESS_STRENGTH, &strength, &max) != AI_SUCCESS)
		strength = 1.0f;

	float shininess = shine * strength;

	std::string textureName;

	if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0) 
	{

		aiString texPath;
		aiReturn texFound = mat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath); // AI_SUCCESS

		size_t found = path.find_last_of("/\\");
		textureName = path.substr(0, found + 1) + texPath.data;

		std::cout << "Loading texture file: " << textureName << std::endl;
		this->material = new MaterialMap(ambient, diffuse, specular, shininess, textureName);
	}
	else 
	{
		this->material = new Material(ambient, diffuse, specular, shininess);
	}


	this->numPrimitives = m->mNumFaces;
	this->numVertices = m->mNumVertices;
	this->vertexSetSize = m->mNumVertices * sizeof(float);
}

void OBJMesh::draw() const
{
	shader->setMaterial(material);
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, numPrimitives * 3, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	for (const auto& m : subMeshes)
	{
		m->draw();
	}
}
