#pragma once

//vulkan, glfw
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//glm
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

//STL
#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <chrono>
#include <memory>

//ubm-vk
#include "Vertex.h"

const std::vector<Vertex> squareVertices = {
	{ { -0.5f, -0.5f },{ 1.0f, 0.0f, 0.0f } },
	{ { 0.5f, -0.5f },{ 0.0f, 1.0f, 0.0f } },
	{ { 0.5f, 0.5f },{ 0.0f, 0.0f, 1.0f } },
	{ { -0.5f, 0.5f },{ 1.0f, 1.0f, 1.0f } }
};

const std::vector<uint16_t> squareIndices = { 0, 1, 2, 2, 3, 0 };

struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

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

#ifdef _DEBUG
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

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

const int MAX_CONCURRENT_FRAMES = 2;

struct QueueFamilyIndices {
	int graphicsFamily = -1;
	int presentFamily = -1;

	bool isComplete() {
		return graphicsFamily >= 0 && presentFamily >= 0;
	}
};

static std::vector<char> readShaderFile(const std::string& filename) {
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

struct SwapchainData {
	std::vector<VkImage> images;
	std::vector<VkImageView> imageViews;
	std::vector<VkFramebuffer> framebuffers;
	VkFormat imageFormat;
	VkExtent2D extent;
};

class RenderSystem
{
private:
	std::shared_ptr<std::vector<GLFWwindow*>> mWindows;
	GLFWwindow * mWindow = nullptr;
	VkInstance mInstance;
	VkDebugReportCallbackEXT callback;

	//Devices and Queues
	VkPhysicalDevice mPhysicalDevice;
	QueueFamilyIndices mSelectedIndices;
	VkDevice mDevice;
	VkQueue mGraphicsQueue;
	VkQueue mPresentQueue;

	//Presentation Surfaces (one for each window)
	std::vector<VkSurfaceKHR> mSurfaces;
	//VkSurfaceKHR mSurface;

	//Swapchain Setup
	std::vector<VkSwapchainKHR> mSwapchains;
	std::vector<SwapchainData> mSwapchainData;

	//std::vector<VkImage> mSwapchainImages;
	//std::vector<VkImageView> mSwapchainImageViews;
	//VkFormat mSwapchainImageFormat;
	//VkExtent2D mSwapchainExtent;
	//std::vector<VkFramebuffer> mSwapchainFramebuffers;
	
	//Pipeline
	VkRenderPass mRenderPass;
	VkDescriptorSetLayout mDescriptorSetLayout;
	VkDescriptorPool mDescriptorPool;
	VkDescriptorSet mDescriptorSet;
	VkPipelineLayout mPipelineLayout;
	VkPipeline mPipeline;

	//Command Buffers
	VkCommandPool mCommandPool;
	std::vector<VkCommandBuffer> mCommandBuffers;

	//Rendering Synchronization primitives
	std::vector<VkSemaphore> mImageAvailableSemaphores;
	std::vector<VkSemaphore> mRenderFinishedSemaphores;
	std::vector<VkFence> mFrameFences;
	size_t mCurrentFrame = 0;

	VkClearValue mClearColor = { 0.0f, 0.5f, 0.5f, 1.0f };
	glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));

	VkBuffer mVertexBuffer;
	VkDeviceMemory mVertexBufferMemory;

	VkBuffer mIndexBuffer;
	VkDeviceMemory mIndexBufferMemory;

	VkBuffer mUniformBuffer;
	VkDeviceMemory mUniformBufferMemory;

public:
	void init(std::shared_ptr<std::vector<GLFWwindow *>> windows);
	void drawFrame();
	void shutdown();

	void setClearColor(VkClearValue clearColor);
private:
	void createInstance();
	void createDevice();
	void createSurfaces();
	
	//SWAPCHAIN
	void createSwapchains();
	VkSurfaceFormatKHR chooseSwapchainSurfaceFormat(VkSurfaceKHR surface);
	VkPresentModeKHR chooseSwapchainPresentMode(VkSurfaceKHR surface);
	VkExtent2D chooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	void createSwapchainImageViews(SwapchainData swapchainData);
	void recreateSwapchain();
	void cleanupSwapchain();

	//Pipeline
	void createGraphicsPipeline();
	VkShaderModule createShaderModule(const std::vector<char>& code);
	void createRenderPass();

	void createDescriptorSetLayout();
	void createDescriptorPool();
	void createDescriptorSet();

	//Command Buffers
	void createFramebuffers();
	void createCommandPool();
	void createCommandBuffers();

	void createSyncObjects();

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	std::vector<const char*> getRequiredExtensions();
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);


	/*Buffer Management*/
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void createVertexBuffer();
	void createIndexBuffer();
	void createUniformBuffer();

	void updateUniformBuffer();
	
	/*
	* Validation Layers
	*/ 
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

	void printExtensions();
	void printPhysicalDeviceDetails(VkPhysicalDevice physicalDevice);
};
