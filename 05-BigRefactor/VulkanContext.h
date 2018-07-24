#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>
#include <vector>

#include "Validation.h"
#include "Extensions.h"
#include "QueueFamilies.h"

//this data frequently gets used together, so i am putting them
//together in a convenient struct
class VulkanContext
{
public:
	GLFWwindow * window;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	QueueFamilyIndices selectedIndices;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkSurfaceKHR surface;

	VulkanContext();

	void initialize(GLFWwindow *window);
	void cleanup();
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

private:
	VkInstance mInstance;
	VkDebugReportCallbackEXT mCallback;

	void createInstance();
	void createDevice(VkInstance instance);
	void createSurface(VkInstance instance, GLFWwindow* window);
	void printPhysicalDeviceDetails(VkPhysicalDevice device);
	std::vector<const char*> getRequiredExtensions();
	void setupDebugCallback();
};