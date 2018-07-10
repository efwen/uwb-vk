#pragma once

#include "DeviceContext.h"

#include <memory>
#include <exception>
#include <vector>

class CommandPool
{
public:
	CommandPool(std::shared_ptr<DeviceContext> context);

	void initialize();

	void cleanup();

	void allocateCommandBuffers(std::vector<VkCommandBuffer> &commandBuffers, VkCommandBufferLevel level);
	void freeCommandBuffers(std::vector<VkCommandBuffer> &commandBuffers);

	VkCommandBuffer beginSingleCmdBuffer();
	void endSingleCmdBuffer(VkCommandBuffer commandBuffer);
private:
	std::shared_ptr<DeviceContext> mContext;
	VkCommandPool mCommandPool;
};