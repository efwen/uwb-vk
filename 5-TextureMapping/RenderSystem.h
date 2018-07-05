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

//ubm-vk
#include "FileIO.h"
#include "Validation.h"
#include "Texture.h"
#include "Vertex.h"
#include "QueueFamilies.h"

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

const int MAX_CONCURRENT_FRAMES = 2;

class RenderSystem
{
public:
	void init(GLFWwindow *window);
	void drawFrame();
	void shutdown();

	void setClearColor(VkClearValue clearColor);

	//Buffer Management
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	//Utility
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

private:
	GLFWwindow * mWindow = nullptr;
	VkInstance mInstance;
	VkDebugReportCallbackEXT mCallback;

	//Devices and Queues
	VkPhysicalDevice mPhysicalDevice;
	QueueFamilyIndices mSelectedIndices;
	VkDevice mDevice;
	VkQueue mGraphicsQueue;
	VkQueue mPresentQueue;

	//Presentation Surface
	VkSurfaceKHR mSurface;

	//Swapchain Setup
	VkSwapchainKHR mSwapchain;
	std::vector<VkImage> mSwapchainImages;
	std::vector<VkImageView> mSwapchainImageViews;
	VkFormat mSwapchainImageFormat;
	VkExtent2D mSwapchainExtent;
	std::vector<VkFramebuffer> mSwapchainFramebuffers;
	
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

	Texture* mTexture;



private:
	void createInstance();
	void createDevice();
	void createSurface(GLFWwindow* window);
	
	//SWAPCHAIN
	void createSwapchain();
	VkSurfaceFormatKHR chooseSwapchainSurfaceFormat();
	VkPresentModeKHR chooseSwapchainPresentMode();
	VkExtent2D chooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	void createSwapchainImageViews();
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

	std::vector<const char*> getRequiredExtensions();
	VkCommandBuffer beginSingleCmdBuffer();
	void endSingleCmdBuffer(VkCommandBuffer commandBuffer);


	/*Buffer Management*/
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void createVertexBuffer();
	void createIndexBuffer();
	void createUniformBuffer();

	void updateUniformBuffer();
	void createTexture();

	void printExtensions();
	void printPhysicalDeviceDetails(VkPhysicalDevice physicalDevice);

	void setupDebugCallback();
};
