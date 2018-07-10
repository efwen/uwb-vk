#pragma once

//vulkan, glfw
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//glm
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
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
#include "Extensions.h"
#include "VulkanContext.h"
#include "QueueFamilies.h"
#include "CommandPool.h"
#include "BufferManager.h"
#include "Swapchain.h"
#include "Texture.h"
#include "Vertex.h"



const std::vector<Vertex> squareVertices = {
	{ { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
	{ { 0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
	{ { 0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
	{ { -0.5f, 0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } }
};

const std::vector<uint16_t> squareIndices = { 0, 1, 2, 2, 3, 0 };

struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};



const int MAX_CONCURRENT_FRAMES = 2;

class RenderSystem
{
public:
	RenderSystem() {}
	~RenderSystem() {}

	void initialize(GLFWwindow *window);
	void drawFrame();
	void cleanup();

	void setClearColor(VkClearValue clearColor);

	/* Image Management */
	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage & image, VkDeviceMemory & imageMemory);
	VkImageView createImageView(VkImage image, VkFormat imageFormat);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

private:
	VkInstance mInstance;
	VkDebugReportCallbackEXT mCallback;

	std::shared_ptr<VulkanContext> mContext;
	std::shared_ptr<BufferManager> mBufferManager;

	//Presentation Surface
	VkSurfaceKHR mSurface;

	//Swapchain Setup
	std::unique_ptr<Swapchain> mSwapchain;

#pragma region Pipeline
	//more closely attached to a renderpass than swapchain
	std::vector<VkFramebuffer> mSwapchainFramebuffers;
	
	//Pipeline
	VkRenderPass mRenderPass;
	VkDescriptorSetLayout mDescriptorSetLayout;
	VkDescriptorPool mDescriptorPool;
	std::vector<VkDescriptorSet> mDescriptorSets;
	VkPipelineLayout mPipelineLayout;
	VkPipeline mPipeline;
#pragma endregion

	//Command Buffers
	std::shared_ptr<CommandPool> mCommandPool;
	//VkCommandPool mCommandPool;
	std::vector<VkCommandBuffer> mCommandBuffers;

	//Rendering Synchronization primitives
	std::vector<VkSemaphore> mImageAvailableSemaphores;
	std::vector<VkSemaphore> mRenderFinishedSemaphores;
	std::vector<VkFence> mFrameFences;
	size_t mCurrentFrame = 0;

	VkClearValue mClearColor = { 0.0f, 0.5f, 0.5f, 1.0f };
	glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));

#pragma region Buffers
	VkBuffer mVertexBuffer;
	VkDeviceMemory mVertexBufferMemory;

	VkBuffer mIndexBuffer;
	VkDeviceMemory mIndexBufferMemory;

	std::vector<VkBuffer> mUniformBuffers;
	std::vector<VkDeviceMemory> mUniformBuffersMemory;
#pragma endregion

	Texture* mTexture;

private:
	void createInstance();
	void createDevice();
	void createSurface(GLFWwindow* window);
	
	//SWAPCHAIN
	void createSwapchain();
	void recreateSwapchain();
	void cleanupSwapchain();

	//Pipeline
	void createGraphicsPipeline();
	VkShaderModule createShaderModule(const std::vector<char>& code);
	void createRenderPass();

	void createDescriptorSetLayout();
	void createDescriptorPool();
	void createDescriptorSets();

	//Command Buffers
	void createFramebuffers();
	void createCommandPool();
	void createCommandBuffers();

	void createSyncObjects();

	/*Buffer Management*/
	void createBuffers();

	void createVertexBuffer();
	void createIndexBuffer();
	void createUniformBuffers();
	void updateUniformBuffer(uint32_t currentImage);
	
	void createTexture();

	//Utility
	std::vector<const char*> getRequiredExtensions();
	void printExtensions();
	void setupDebugCallback();
};