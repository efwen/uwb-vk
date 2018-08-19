#pragma once

#include <vulkan/vulkan.h>
#include <exception>
#include <vector>

#include "VulkanContext.h"
#include "CommandPool.h"
#include "Vertex.h"
#include "UBO.h"

/**
	@class BufferManager

	@brief Subsystem in charge of buffer-related operations

	This class Manages the creation of various types of buffers, as well as some
	operations such as copying.

	@author Nicholas Carpenetti

	@date 10 August 2018
*/
class BufferManager
{
public:

	/** @brief BufferManager Constructor
		@param context The Vulkan Context for this application
		@param commandPool The Command Pool object. Needed because some buffer 
			operations make use of command buffers
	*/
	BufferManager(std::shared_ptr<VulkanContext> context, std::shared_ptr<CommandPool> commandPool);
	~BufferManager();

	//-----------------------------------------------------------------------------------------------

	/** @brief Creates a buffer for holding vertex information
		@param vertices Vector of vertices to be placed in the buffer
		@param vertexBuffer Handle to the buffer to be set
		@param vertexBufferMemory handle to the buffer memory to be set
	*/
	void createVertexBuffer(const std::vector<Vertex>& vertices, VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory);

	/** @brief Creates a buffer for holding index information
		@param vertices Vector of indices to be placed in the buffer
		@param vertexBuffer Handle to the buffer to be set
		@param vertexBufferMemory handle to the buffer memory to be set
	*/
	void createIndexBuffer(const std::vector<uint32_t>& indices, VkBuffer& indexBuffer, VkDeviceMemory& indexBufferMemory);


	/** @brief Copies information from a VkBuffer object to a VkImage object
		@param buffer Handle to the VkBuffer object to be copied
		@param image  Handle to the destination VkImage
		@param width  Width of the destination VkImage
		@param height Height of the destination VkImage
	*/
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	

	/** @brief Create a new VkBuffer object
		@param size the memory size of the buffer to be created (in bytes)
		@param usage Usage flags for the buffer
		@param properties Properties of the buffer to be created
		@param buffer The VkBuffer handle to be created
		@param bufferMemory The VKBufferMemory handle to be created
	*/
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	
	/** @brief Copy a VkBufferObject
		@param srcBuffer VkBuffer copy source
		@param dstBuffer VkBuffer copy destination
		@param size the memory size of the source buffer (in bytes)
	*/
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

private:
	std::shared_ptr<VulkanContext> mContext;	///< A pointer to the Vulkan Context
	std::shared_ptr<CommandPool> mCommandPool;	///< A poitner to the Command Pool
};
