#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>
#include <vector>

#include "QueueFamilies.h"
#include "Extensions.h"

//this data frequently gets used together, so i am putting them
//together in a convenient struct
class VulkanContext
{
public:
	GLFWwindow * window = nullptr;
	VkInstance instance = VK_NULL_HANDLE;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	QueueFamilyIndices selectedIndices;
	VkQueue graphicsQueue = VK_NULL_HANDLE;
	VkQueue presentQueue = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;

	void initialize(VkInstance instance, GLFWwindow *window)
	{
		this->window = window;
		createSurface(instance, window);
		createDevice(instance);
	}

	void cleanup(VkInstance instance)
	{
		vkDestroyDevice(device, nullptr);
		vkDestroySurfaceKHR(instance, surface, nullptr);
	}

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("Failed to find a suitable memory type!");
	}

private:
	void createDevice(VkInstance instance)
	{
		std::cout << "Creating Device" << std::endl;

		//get a list of physical devices
		uint32_t physicalDeviceCount;
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);

		std::vector<VkPhysicalDevice> availableDevices(physicalDeviceCount);
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, availableDevices.data());

		//select one of the devices
		std::cout << "Devices Found:" << physicalDeviceCount << std::endl;
		for (const auto& device : availableDevices) {
			printPhysicalDeviceDetails(device);
		}

		//just pick the first physical device for now
		//we'll need to do device suitability checks later
		physicalDevice = availableDevices[0];

		selectedIndices = findQueueFamilies(physicalDevice, surface);

		//create a queue for both graphics and presentation
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

		//create a graphics queue
		VkDeviceQueueCreateInfo graphicsQueueCreateInfo = {};
		graphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		graphicsQueueCreateInfo.queueFamilyIndex = selectedIndices.graphicsFamily;
		graphicsQueueCreateInfo.queueCount = 1;
		float queuePriority = 1.0f;
		graphicsQueueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(graphicsQueueCreateInfo);

		//create a separate present queue if necessary (different family)
		if (selectedIndices.graphicsFamily != selectedIndices.presentFamily) {
			VkDeviceQueueCreateInfo presentQueueCreateInfo = {};
			presentQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			presentQueueCreateInfo.queueFamilyIndex = selectedIndices.presentFamily;
			presentQueueCreateInfo.queueCount = 1;
			float queuePriority = 1.0f;
			presentQueueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(presentQueueCreateInfo);
		}

		//specify the features of the device we'll be using
		VkPhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		//main createInfo struct
		VkDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

		//extensions we want this device to use
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

		//create the device
		if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS) {
			std::runtime_error("Device Creation Failed");
		}

		//get the queue handles
		vkGetDeviceQueue(device, selectedIndices.graphicsFamily, 0, &graphicsQueue);
		vkGetDeviceQueue(device, selectedIndices.presentFamily, 0, &presentQueue);
	}

	void createSurface(VkInstance instance, GLFWwindow* window)
	{
		std::cout << "Creating Surface" << std::endl;

		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create a window surface!");
		}
	}

	void printPhysicalDeviceDetails(VkPhysicalDevice device)
	{
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(device, &properties);

		std::cout << properties.deviceName << std::endl;
		std::cout << "API Version: " << properties.apiVersion << std::endl;
	}
};