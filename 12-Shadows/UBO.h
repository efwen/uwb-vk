#pragma once

#include <vulkan/vulkan.h>
#include <vector>

struct UBO
{
	VkDeviceSize bufferSize = 0;
	std::vector<VkBuffer> buffers;
	std::vector<VkDeviceMemory> buffersMemory;
};