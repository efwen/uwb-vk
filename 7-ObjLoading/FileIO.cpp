#include "FileIO.h"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

//#define TINYOBJLOADER_IMPLEMENTATION
//#include <tiny_obj_loader.h>

std::vector<char> readShaderFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("Unable to open shader file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();
	return buffer;
}

//Read in a .obj file and convert it into a vertex and index buffer
//Current assumptions:
//Triangles only
void readObjFile(const std::string& filename, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
	std::ifstream file(filename);
	std::string cmd;
	std::string line;

	std::vector<glm::vec2> texCoords;

	std::cout << "Reading obj file \"" << filename << "\"" << std::endl;
	while (std::getline(file, line)) {
		if (line == "" || line[0] == '#')	//ignore empty lines and comments
			continue;

		std::istringstream linestr(line);
		linestr >> cmd;
		if (cmd == "v") //vertex definition
		{
			Vertex vertex;
			linestr >> vertex.pos.x >> vertex.pos.y >> vertex.pos.z;
			
			vertex.color = { 1.0, 1.0, 1.0 };
			vertex.texCoord = { -1.0f, -1.0f };
			vertices.push_back(vertex);
		}
		else if (cmd == "vt")
		{
			glm::vec2 texCoord;
			linestr >> texCoord.x >> texCoord.y;
			texCoord.y = 1.0f - texCoord.y;

			texCoords.push_back(texCoord);
		}
		else if (cmd == "f")	//format of f index/tecCoordIndex/normalIndex index/tecCoordIndex/normalIndex index/tecCoordIndex/normalIndex ...
		{
			//currently assumes triangles only
			std::array<std::string, 3> faceVertices;
			linestr >> faceVertices[0] >> faceVertices[1] >> faceVertices[2];

			//split by "/"
			for(size_t i = 0; i < faceVertices.size(); i++)
			{
				std::vector<std::string> details = getVertexDetails(faceVertices[i]);
				if (details.size() < 1)
					throw std::runtime_error("Invalid vertex details size!");

				//get the index corresponding to the vertex we want (-1 because obj files use >= 1 for indexing)
				uint32_t index = std::stoi(details[0]) - 1;
				if (index < 0 || index >= vertices.size())
					throw std::runtime_error("Invalid index! ");

				//get texture coordinate (optional)
				uint32_t vt;
				if (details.size() > 1 && details[1] != "")	//if a texture coordinate was provided
				{
					vt = std::stoi(details[1]) - 1; // (-1 again because obj files use >= 1 for indexing)

					if (vertices[index].texCoord.x != -1.0f && vertices[index].texCoord.y != -1.0f)
					{
						if (vertices[index].texCoord != texCoords[vt])
						{
							//we are forced to construct a new vertex to accomodate the different texture coordinate
							Vertex vertex = vertices[index];
							vertex.texCoord = texCoords[vt];
							vertices.push_back(vertex);
							indices.push_back(vertices.size() - 1);
							continue;
						}
					}
					vertices[index].texCoord = texCoords[vt];
				}
				
				indices.push_back(index);
			}
		}
	}
	std::cout << "done loading! (" << vertices.size() << " vertices, " << indices.size() << " indices)" << std::endl;
}

std::vector<std::string> getVertexDetails(const std::string& vertexString)
{
	char delimiter = '/';
	std::vector<std::string> tokens;
	std::istringstream tokenStream(vertexString);
	std::string token;

	while (std::getline(tokenStream, token, delimiter))
	{
		tokens.push_back(token);
	}
	return tokens;
}