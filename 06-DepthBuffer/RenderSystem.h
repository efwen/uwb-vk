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
#include "ImageManager.h"
#include "Swapchain.h"
#include "Texture.h"
#include "Vertex.h"



const std::vector<Vertex> doubleSquareVertices = {
	{ { -0.5f, -0.5f, 0.0f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
	{ { 0.5f, -0.5f, 0.0f },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 0.0f } },
	{ { 0.5f, 0.5f, 0.0f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 1.0f } },
	{ { -0.5f, 0.5f, 0.0f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

	{ { -0.5f, -0.5f, -0.5f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
	{ { 0.5f, -0.5f, -0.5f },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 0.0f } },
	{ { 0.5f, 0.5f, -0.5f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 1.0f } },
	{ { -0.5f, 0.5f, -0.5f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } }
};

const std::vector<uint16_t> doubleSquareIndices = {
	0, 1, 2, 2, 3, 0,
	4, 5, 6, 6, 7, 4
};

struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

const int MAX_CONCURRENT_FRAMES = 2;
const std::string VERT_SHADER_PATH = "Resources/Shaders/textureMapping_vert.spv";
const std::string FRAG_SHADER_PATH = "Resources/Shaders/textureMapping_frag.spv";
const std::string TEXTURE_PATH = "Resources/Textures/UW Logo.png";

class RenderSystem
{
public:
	RenderSystem() {}
	~RenderSystem() {}

	void initialize(GLFWwindow *window);
	void drawFrame();
	void cleanup();

	void setClearColor(VkClearValue clearColor);

private:
	std::shared_ptr<VulkanContext> mContext;
	std::shared_ptr<CommandPool> mCommandPool;
	std::shared_ptr<BufferManager> mBufferManager;
	std::shared_ptr<ImageManager> mImageManager;

	//Swapchain Setup
	std::unique_ptr<Swapchain> mSwapchain;

	//Command Buffers
	std::vector<VkCommandBuffer> mCommandBuffers;

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
	//depth buffer
	VkImage mDepthImage;
	VkFormat mDepthImageFormat;
	VkDeviceMemory mDepthImageMemory;
	VkImageView mDepthImageView;

	//Rendering Synchronization primitives
	std::vector<VkSemaphore> mImageAvailableSemaphores;
	std::vector<VkSemaphore> mRenderFinishedSemaphores;
	std::vector<VkFence> mFrameFences;
	size_t mCurrentFrame = 0;

	VkClearValue mClearColor = { 0.0f, 0.5f, 0.5f, 1.0f };
	glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));

#pragma region Buffers
	//vertex buffer
	VkBuffer mVertexBuffer;
	VkDeviceMemory mVertexBufferMemory;

	//index buffer
	VkBuffer mIndexBuffer;
	VkDeviceMemory mIndexBufferMemory;

	//ubo
	std::vector<VkBuffer> mUniformBuffers;
	std::vector<VkDeviceMemory> mUniformBuffersMemory;
#pragma endregion

	Texture* mTexture;

private:
	//void createInstance();
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

	//Descriptors
	void createDescriptorSetLayout();
	void createDescriptorPool();
	void createDescriptorSets();

	//Command Buffers
	void createFramebuffers();
	void createCommandPool();
	void createCommandBuffers();
	void createDepthBuffer();
	void createSyncObjects();


	void createMesh(const std::vector<Vertex> &vertices, const std::vector<uint16_t> &indices);
	void createTexture(const std::string &filename);
	void createUniformBufferObject();
	void updateUniformBuffer(uint32_t currentImage);
};