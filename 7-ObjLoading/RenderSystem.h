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


struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

const int MAX_CONCURRENT_FRAMES = 2;
const std::string VERT_SHADER_PATH = "shaders/square_vert.spv";
const std::string FRAG_SHADER_PATH = "shaders/square_frag.spv";
const std::string TEXTURE_PATH = "textures/chalet.jpg";
const std::string MODEL_PATH = "models/chalet.obj";

class RenderSystem
{
public:
	RenderSystem() {}
	~RenderSystem() {}

	void initialize(GLFWwindow *window);
	void drawFrame();
	void cleanup();

	void setClearColor(VkClearValue clearColor);
	void setCamDist(float dist);
	float getCamDist();
	glm::vec3* getCamRotate();
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
	float mCamDist = 4.0;
	glm::vec3 mCamRotate = glm::vec3(0.0f);

#pragma region Buffers
	//vertex buffer
	VkBuffer mVertexBuffer;
	VkDeviceMemory mVertexBufferMemory;

	//index buffer
	std::vector<Vertex> mVertices;
	std::vector<uint32_t> mIndices;
	VkBuffer mIndexBuffer;
	VkDeviceMemory mIndexBufferMemory;

	//ubo
	std::vector<VkBuffer> mUniformBuffers;
	std::vector<VkDeviceMemory> mUniformBuffersMemory;
#pragma endregion

	Texture* mTexture;

	//"camera"

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


	void createMesh(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);
	void loadModel(const std::string &filename, std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);
	void createTexture(const std::string &filename);
	void createUniformBufferObject();
	void updateUniformBuffer(uint32_t currentImage);
};