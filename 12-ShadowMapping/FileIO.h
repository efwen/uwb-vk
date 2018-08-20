#pragma once

#include <string>
#include <vector>

#include "Vertex.h"

/** @brief load a shader file
	@param filename The filename of the shader file
*/
std::vector<char> readShaderFile(const std::string& filename);

/** @brief load a .obj file
	@param filename The filename of the .obj file to be loaded
	@param vertices Vertices to load from the .obj file
	@param indices Indices to load the .obj file
	@param frontFace The face the engine considers to be front face (clockwise or counter-clockwise)
*/
void readObjFile(const std::string& filename, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, VkFrontFace frontFace);

/** @brief A helper function for readObjFile. Tokenizes position/texture coordinate/normal into individual strings
	@param vertexString A string containing the combined indices for one vertex
*/
std::vector<std::string> getVertexDetails(const std::string& vertexString);