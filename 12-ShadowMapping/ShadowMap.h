#pragma once

#include <vulkan/vulkan.h>

struct ShadowMap {
	VkImage image = VK_NULL_HANDLE;
	VkFormat imageFormat = VK_FORMAT_UNDEFINED;
	VkDeviceMemory imageMemory = VK_NULL_HANDLE;
	VkImageView imageView = VK_NULL_HANDLE;
	VkSampler imageSampler = VK_NULL_HANDLE;
};