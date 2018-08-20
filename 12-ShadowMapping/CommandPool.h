#pragma once

#include "VulkanContext.h"

#include <memory>
#include <exception>
#include <vector>

/** @class CommandPool

	@brief Allocation and management of Command Buffers

	@author Nicholas Carpenetti

	@date 9 July 2018
*/
class CommandPool
{
public:
	/** @brief Constructor
		@param context The Vulkan Context to operate within
	*/
	CommandPool(std::shared_ptr<VulkanContext> context);
	/** @brief Initialize by creating a VkCommandPool
	*/
	void initialize();
	/** @brief Cleanup by destroying the VkCommandPool
	*/
	void cleanup();


	/** @brief Allocate command buffers from the pool
		@param commandBuffers The set of command buffers to be allocated
		@param level The level of the command buffers (primary or secondary)
	*/
	void allocateCommandBuffers(std::vector<VkCommandBuffer> &commandBuffers, VkCommandBufferLevel level);
	/**
		@brief Free a set of command buffers
		@param commandBuffers The set of command buffers to be freed
	*/
	void freeCommandBuffers(std::vector<VkCommandBuffer> &commandBuffers);


	/** @brief Begin a command buffer
	*/
	VkCommandBuffer beginSingleCmdBuffer();
	/** @brief End a particular command buffer
	*/
	void endSingleCmdBuffer(VkCommandBuffer commandBuffer);
private:
	std::shared_ptr<VulkanContext> mContext;	///< A reference to the Vulkan Context
	VkCommandPool mCommandPool;					///< The Command Pool
};