#pragma once

//Vulkan
#include <vulkan/vulkan.h>

/**
	@brief A Set of Queue Family indices
	
	Each physicalDevice has several queue families, each with different
	properties. Differengt queue families must be used for different
	purposes. This struct holds the indices of the families that are
	suited for each type of way we use the queue. 
*/
struct QueueFamilyIndices {
	int graphicsFamily = -1;		///< The queue familiy used for graphics operations
	int presentFamily = -1;			///< The queue familiy used for presentation operations

	/** @brief Determine if all the queue families have been found
	*/
	bool isComplete() {
		return graphicsFamily >= 0 && presentFamily >= 0;
	}
};

/** @brief Find a queue family of each type from a particular physical device and surface
	@param physicalDevice The physical device used to render
	@param surface The surface being rendered to

	@return A QueueFamilyIndices struct containing the selected queues
*/
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);