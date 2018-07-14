#pragma once

//STL
#include <string>
#include <vector>

#include "Vertex.h"

std::vector<char> readShaderFile(const std::string& filename);

void readObjFile(const std::string& filename, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);

//helper function for readObjFile
std::vector<std::string> getVertexDetails(const std::string& vertexString);