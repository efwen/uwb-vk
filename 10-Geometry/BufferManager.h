#pragma once

#include <vulkan/vulkan.h>
#include <exception>
#include <vector>

#include "VulkanContext.h"
#include "CommandPool.h"
#include "Vertex.h"
#include "UBO.h"

/*
MemoryManager
Handles all of the memory operations for the RenderSystem
*/

class BufferManager
{
public:
	/*
	BufferManager
	Constructor for the Buffer Manager.
	Assumptions:
		context is valid and complete
		commandPool is a valid handle to a command pool associated with the context
	*/
	BufferManager(std::shared_ptr<VulkanContext> context, std::shared_ptr<CommandPool> commandPool);
	~BufferManager();

	/* Buffer Management */
	void createVertexBuffer(const std::vector<Vertex>& vertices, VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory);
	void createIndexBuffer(const std::vector<uint32_t>& indices, VkBuffer& indexBuffer, VkDeviceMemory& indexBufferMemory);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	
	/*Buffer Management*/
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

private:
	std::shared_ptr<VulkanContext> mContext;
	std::shared_ptr<CommandPool> mCommandPool;

	//Helper functions
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};
