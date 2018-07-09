#pragma once

#include <vulkan/vulkan.h>
#include "QueueFamilies.h"

//this data frequently gets used together, so i am putting them
//together in a convenient struct
struct DeviceContext
{
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	QueueFamilyIndices selectedIndices;
	VkQueue graphicsQueue = VK_NULL_HANDLE;
	VkQueue presentQueue = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;	// maybe / maybe not
};