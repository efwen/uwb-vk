#pragma once

#include <vulkan/vulkan.h>

/** @brief A ShadowMap

	A ShadowMap. Very similar to texture in data used, but its context of
	use is very different, requiring its own struct.
*/
struct ShadowMap {
	VkImage image = VK_NULL_HANDLE;
	VkFormat imageFormat = VK_FORMAT_UNDEFINED;
	VkDeviceMemory imageMemory = VK_NULL_HANDLE;
	VkImageView imageView = VK_NULL_HANDLE;
	VkSampler imageSampler = VK_NULL_HANDLE;
};