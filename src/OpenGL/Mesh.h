#pragma once

#include "TShader.h"

#include <glm/glm.hpp>


#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <vector>


struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	glm::vec3 Tangent;
	glm::vec3 Bitangent;
};

struct Texture {
	unsigned int id;
	std::string type;
	aiString path;  // ���Ǵ��������·������������������бȽ�
	//~Texture()
	//{
	//	glDeleteTextures(1, &id);
	//}
};

class Mesh {
public:
	/*  ��������  */
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	/*  ����  */
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
	//~Mesh();

	void Draw(const TShader &shader);
private:

	/*  ��Ⱦ����  */
	unsigned int VAO, VBO, EBO;

	/*  ����  */
	void setupMesh();
};

