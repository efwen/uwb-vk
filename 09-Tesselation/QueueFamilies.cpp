#include "QueueFamilies.h"

//STL
#include <vector>
#include <iostream>

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	QueueFamilyIndices indices;

	//get the queue families for this device
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	std::cout << "--------------------------------" << std::endl;
	std::cout << queueFamilyCount << " queue families found" << std::endl;

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		std::cout << "Family " << i << ":" << std::endl;
		std::cout << queueFamily.queueCount << " queues" << std::endl;
		std::cout << "flags:" << std::endl;
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			std::cout << "-graphics" << std::endl;
			indices.graphicsFamily = i;
		}
		if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
			std::cout << "-compute" << std::endl;
		}
		if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) {
			std::cout << "-transfer" << std::endl;
		}
		if (queueFamily.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) {
			std::cout << "-sparse-binding" << std::endl;
		}
		if (queueFamily.queueFlags & VK_QUEUE_PROTECTED_BIT) {
			std::cout << "-protected" << std::endl;
		}

		//figure out if this queue family supports presentation
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
		if (queueFamily.queueCount > 0 && presentSupport) {
			indices.presentFamily = i;
		}

		//if we have a present queue for both graphics and presentation we're done
		if (indices.isComplete())
			break;

		i++;
	}
	std::cout << "--------------------------------" << std::endl;
	return indices;
};