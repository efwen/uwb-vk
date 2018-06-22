#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <vector>


//Validation Layers
const std::vector<const char*> validationLayers = {
	"VK_LAYER_LUNARG_standard_validation"
};

//Instance Extensions
const std::vector<const char*> instanceExtensions = {
	VK_KHR_SURFACE_EXTENSION_NAME
#ifdef _DEBUG
	VK_EXT_DEBUG_REPORT_EXTENSION_NAME
#endif
};

//Device Extensions
const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

/**	\brief A debug callback for the reporting validation layer messages
*
*	\param flags		[fill]
*	\param objectType	[fill]
*	\param object		[fill]
*	\param location		[fill]
*	\param messageCode	[fill]
*	\param pLayerPrefix	A prefix representing The validation Layer the message came from
*	\param pMessage		The message to display
*	\param pUserData	User data sent with the debug message
*/
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t obj,
	size_t location,
	int32_t code,
	const char* layerPrefix,
	const char* msg,
	void* userData) {

	std::cerr << "validation layer: " << msg << std::endl;

	return VK_FALSE;
}

#ifndef DEBUG
	const bool enableValidationLayers = true;
#else
	const bool enableValidationLayers = false;
#endif

struct QueueFamilyIndices {
	int graphicsFamily = -1;
	int presentFamily = -1;

	bool isComplete() {
		return graphicsFamily >= 0 && presentFamily >= 0;
	}
};

std::vector<char> readShaderFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("Unable to open shader file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	
	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();
	return buffer;
}

class RenderSystem
{
private:
	VkInstance mInstance;
	VkDebugReportCallbackEXT callback;
	VkPhysicalDevice mPhysicalDevice;
	QueueFamilyIndices mSelectedIndices;
	VkDevice mDevice;
	VkSurfaceKHR mSurface;

	VkQueue mGraphicsQueue;
	VkQueue mPresentQueue;

	VkSwapchainKHR mSwapchain;
	std::vector<VkImage> mSwapchainImages;
	std::vector<VkImageView> mSwapchainImageViews;
	VkFormat mSwapchainImageFormat;
	VkExtent2D mSwapchainExtent;
	
	VkCommandPool mCommandPool;
	VkCommandBuffer mBuffer;

public:
	void init(GLFWwindow *window);
	void update();
	void shutdown();
private:
	void createInstance();
	void createDevice();
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	void createSurface(GLFWwindow* window);
	
	//SWAPCHAIN
	void createSwapchain();
	VkSurfaceFormatKHR chooseSwapchainSurfaceFormat();
	VkPresentModeKHR chooseSwapchainPresentMode();
	VkExtent2D chooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	void createSwapchainImageViews();

	//Pipeline
	void createGraphicsPipeline();


	std::vector<const char*> getRequiredExtensions();

	//validation layers
	void createDebugCallback();
	void destroyDebugCallback();

	VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) {
		auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pCallback);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	//utility
	void printExtensions();
	void printPhysicalDeviceDetails(VkPhysicalDevice physicalDevice);
};
