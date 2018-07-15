#pragma once

//Vulkan
#include <vulkan/vulkan.h>

//STL
#include <vector>
#include <memory>

//uwb-vk
#include "Texture.h"

//Model: a renderable object
class Model
{
public:
	//vertex buffer
	std::vector<Vertex> mVertices;
	VkBuffer mVertexBuffer;
	VkDeviceMemory mVertexBufferMemory;

	//index buffer
	std::vector<uint32_t> mIndices;
	VkBuffer mIndexBuffer;
	VkDeviceMemory mIndexBufferMemory;

	std::shared_ptr<Texture> mTexture;
};