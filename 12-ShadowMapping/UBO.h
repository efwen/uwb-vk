#pragma once

#include <vulkan/vulkan.h>
#include <vector>

/** @class UBO
	
	@brief A Uniform Buffer Object for sending data to shaders
*/
struct UBO
{
	VkDeviceSize bufferSize = 0;					///< The Size of the objects in the buffer (in bytes)
	std::vector<VkBuffer> buffers;					///< The VkBuffers that are used in the UBO
	std::vector<VkDeviceMemory> buffersMemory;		///< The device memory used to make the UBO
};