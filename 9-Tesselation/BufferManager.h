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
	
	//UniformBufferObjects
	//functions must be defined here due to templates
	template<typename T>
	void createUBO(UBO& ubo, uint32_t swapchainSize)
	{
		VkDeviceSize bufferSize = sizeof(T);

		ubo.buffers.resize(swapchainSize);
		ubo.buffersMemory.resize(swapchainSize);

		for (size_t i = 0; i < swapchainSize; i++) {
			createBuffer(bufferSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				ubo.buffers[i],
				ubo.buffersMemory[i]);
		}
	}

	template<typename T>
	void updateUBO(UBO& ubo, T& uboData, uint32_t index)
	{
		void* data;
		vkMapMemory(mContext->device, ubo.buffersMemory[index], 0, sizeof(uboData), 0, &data);
		memcpy(data, &uboData, sizeof(T));
		vkUnmapMemory(mContext->device, ubo.buffersMemory[index]);
	}

	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	
	/*Buffer Management*/
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

private:
	std::shared_ptr<VulkanContext> mContext;
	std::shared_ptr<CommandPool> mCommandPool;

	//Helper functions
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	//VkCommandBuffer beginSingleCmdBuffer();
	//void endSingleCmdBuffer(VkCommandBuffer commandBuffer);
};
