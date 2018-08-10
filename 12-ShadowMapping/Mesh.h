#pragma once

//Vulkan
#include <vulkan/vulkan.h>

//STL
#include <vector>
#include <memory>

//uwb-vk
#include "VulkanContext.h"
#include "BufferManager.h"
#include "Vertex.h"

class Mesh
{
public:
	Mesh(std::shared_ptr<VulkanContext> context, std::shared_ptr<BufferManager> bufferManager);
	~Mesh();
	
	void load(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
	void free();

	uint32_t getIndexCount();
	VkBuffer getVertexBuffer();
	VkBuffer getIndexBuffer();
protected:
	std::shared_ptr<VulkanContext> mContext;
	std::shared_ptr<BufferManager> mBufferManager;

	//vertex buffer
	std::vector<Vertex> mVertices;
	VkBuffer mVertexBuffer;
	VkDeviceMemory mVertexBufferMemory;

	//index buffer
	std::vector<uint32_t> mIndices;
	VkBuffer mIndexBuffer;
	VkDeviceMemory mIndexBufferMemory;
};