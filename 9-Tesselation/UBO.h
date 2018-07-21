#pragma once

#include <vulkan/vulkan.h>
#include <vector>

struct UBO
{
	std::vector<VkBuffer> buffers;
	std::vector<VkDeviceMemory> buffersMemory;
};