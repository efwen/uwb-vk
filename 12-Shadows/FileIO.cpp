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
void readObjFile(const std::string& filename, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, VkFrontFace frontFace)
{
	std::ifstream file(filename);

	if (!file.is_open()) {
		throw std::runtime_error("Unable to open .obj file!");
	}
	std::cout << "Reading obj file \"" << filename << "\"" << std::endl;


	std::unordered_map<Vertex, uint32_t> vertexMap = {};	//to help take advantage of the index buffer
	std::vector<glm::vec3> vertPositions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texCoords;
	
	std::string cmd;
	std::string line;
	while (std::getline(file, line)) {
		if (line == "" || line[0] == '#')	//ignore empty lines and comments
			continue;

		std::istringstream linestr(line);
		linestr >> cmd;
		if (cmd == "v")			//geometric vertex definition
		{
			glm::vec3 pos;
			linestr >> pos.x >> pos.y >> pos.z;
			vertPositions.push_back(pos);
		}
		else if (cmd == "vt")	//texture vertex definition
		{
			glm::vec2 texCoord;
			linestr >> texCoord.x >> texCoord.y;
			texCoord.y = 1.0f - texCoord.y;
			texCoords.push_back(texCoord);
		}
		else if (cmd == "vn")
		{
			glm::vec3 normal;
			linestr >> normal.x >> normal.y >> normal.z;
			normals.push_back(normal);
		}
		//format of f index/tecCoordIndex/normalIndex index/tecCoordIndex/normalIndex index/tecCoordIndex/normalIndex ...
		//counter-clockwise in file format
		else if (cmd == "f")	
		{
			//this is where we actually add the vertex/index
			//currently assumes triangles only
			std::array<std::string, 3> faceVertices;
			if(frontFace == VK_FRONT_FACE_CLOCKWISE)	//if the pipeline is set up to be clockwise, invert the order
				linestr >> faceVertices[2] >> faceVertices[1] >> faceVertices[0];
			else
				linestr >> faceVertices[0] >> faceVertices[1] >> faceVertices[2];

			//split by "/"
			for(size_t i = 0; i < faceVertices.size(); i++)
			{
				std::vector<std::string> details = getVertexDetails(faceVertices[i]);

				if (details.size() < 1)
					throw std::runtime_error("Invalid vertex details size!");

				//get the index corresponding to the vertex we want (subtract 1 because obj files use >= 1 for indexing)
				uint32_t posIndex = std::stoi(details[0]) - 1;
				if (posIndex < 0 || posIndex >= vertPositions.size())
					throw std::runtime_error("Invalid geometry index!");

				int32_t texIndex = -1;
				if (details.size() > 1 && details[1] != "")	//if a texture coordinate was provided
				{
					texIndex = std::stoi(details[1]) - 1; // get the index (-1 again because obj files use >= 1 for indexing)
					if (texIndex < 0 || texIndex >= texCoords.size())
						throw std::runtime_error("Invalid texture index!");
				}

				int32_t normalIndex = -1;
				if (details.size() > 2 && details[2] != "")
				{
					normalIndex = std::stoi(details[2]) - 1;
					if (normalIndex < 0 || normalIndex >= normals.size())
						throw std::runtime_error("Invalid normals index!");
				}

				//make a vertex object, and fill it with data
				Vertex vertex;
				vertex.pos = glm::vec4(vertPositions[posIndex], 1.0);
				vertex.color = { 1.0f, 1.0f, 1.0f , 1.0f };
				vertex.tangent = { 0.0f, 0.0f, 0.0f };
				if (texIndex > -1)
					vertex.texCoord = texCoords[texIndex];
				if (normalIndex > -1)
					vertex.normal = normals[normalIndex];

				if (vertexMap.count(vertex) == 0)	//if this is our first occurence add it to the map
				{
					vertexMap[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}

				indices.push_back(vertexMap[vertex]);
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