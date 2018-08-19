#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>
#include <vector>

#include "Validation.h"
#include "Extensions.h"
#include "QueueFamilies.h"

/**
	@class VulkanContext

	@brief Contains primary Vulkan constructs that remain constant throughout the
		a uwb-vk application.
	
	Initialization of the Vulkan Instance, acquisition of a device, creation
	of a surface to render to, and setting up the validation layers is handled
	here. This is the first Object to be made in RenderSystem, and the last to
	be destroyed.

	@author Nicholas Carpenetti

	@date 8 July 2018
 */
class VulkanContext
{
public:
	GLFWwindow * window;				///< GLFW window the application renders to
	VkPhysicalDevice physicalDevice;	///< Selected physical device
	VkDevice device;					///< Device used for all Vulkan operations
	QueueFamilyIndices selectedIndices;	///< Indices of the selected device queues
	VkQueue graphicsQueue;				///< Queue used for drawing
	VkQueue presentQueue;				///< Queue used for presentation
	VkSurfaceKHR surface;				///< Surface to be drawn to

	VulkanContext();

	/** @brief Initializes the VulkanContext
		@param window the window to be rendered to
		@param appName the name of the application
	*/
	void initialize(GLFWwindow *window, const std::string& appName);

	/** @brief Cleans up all allocated resources 
	*/
	void cleanup();

	/** @brief Finds the best available memory type
		@param typeFilter A bitmask representing the memory types that are requested.
			Found in VkMemoryRequirements.memoryTypeBits
		
		@param properties Properties required of the memory type
	*/
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

private:
	VkInstance mInstance;				///< Vulkan Instance
	VkDebugReportCallbackEXT mCallback;	///< Callback used for validation

	/** @brief Creates the VkInstance Object
		@param appName The name of the application
	*/
	void createInstance(const std::string& appName);

	/** Creates the VkDevice Object
	*/
	void createDevice(VkInstance instance);
	
	/** @brief Creates the VkSurfaceKHR Object
		@param instance The Vulkan Instance

		@param window The GLFW window being used
	*/
	void createSurface(VkInstance instance, GLFWwindow* window);
	
	/** @brief Prints the details of a given VkPhysicalDevice to the console
		@param device The physical device
	*/
	void printPhysicalDeviceDetails(VkPhysicalDevice device);
	
	/** @brief Gets a list of extensions required to run the application
	*/
	std::vector<const char*> getRequiredExtensions();
	
	/** @brief Sets up a callback for Validation Layers */
	void setupDebugCallback();
};