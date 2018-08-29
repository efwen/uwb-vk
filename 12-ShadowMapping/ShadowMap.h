#pragma once

#include <vulkan/vulkan.h>

/** @brief A ShadowMap

	A ShadowMap. Very similar to texture in data used, but its context of
	use is very different, requiring its own struct.
*/
struct ShadowMap {
	VkImage image = VK_NULL_HANDLE;					///< The VkImage object written to
	VkFormat imageFormat = VK_FORMAT_UNDEFINED;		///< The format of the VkImage
	VkDeviceMemory imageMemory = VK_NULL_HANDLE;	///< Handle to the the deviceMemory the VkImage resides in
	VkImageView imageView = VK_NULL_HANDLE;			///< A view to the VkImage object
	VkSampler imageSampler = VK_NULL_HANDLE;		///< A sampler so the ShadowMap can be accessed in future passes
};