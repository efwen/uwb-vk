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
#include "Model.h"


struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

const std::string VERT_SHADER_PATH = "shaders/square_vert.spv";
const std::string FRAG_SHADER_PATH = "shaders/square_frag.spv";
const int MAX_CONCURRENT_FRAMES = 2;

class RenderSystem
{
public:
	RenderSystem() {}
	~RenderSystem() {}

	void initialize(GLFWwindow *window);
	void drawFrame();
	void cleanup();

	//create a Model to render (probably better to use Mesh + Texture objects for reuse
	std::shared_ptr<Model> createModel(const std::string& meshFile, const std::string& textureFile);
	void cleanupModel(std::shared_ptr<Model> model);


	void setClearColor(VkClearValue clearColor);
	void setCamDist(float dist);
	float getCamDist();
	glm::vec3* getCamRotate();



	//std::shared_ptr<Model> mGroundModel;
	//std::shared_ptr<Model> mChaletModel;
	std::vector<std::shared_ptr<Model>> modelList;
private:
	std::shared_ptr<VulkanContext> mContext;
	std::shared_ptr<CommandPool> mCommandPool;
	std::shared_ptr<BufferManager> mBufferManager;
	std::shared_ptr<ImageManager> mImageManager;

	std::unique_ptr<Swapchain> mSwapchain;
	std::vector<VkCommandBuffer> mCommandBuffers;

#pragma region Pipeline
	//more closely attached to a renderpass than swapchain
	std::vector<VkFramebuffer> mSwapchainFramebuffers;
	
	//Pipeline
	VkRenderPass mRenderPass;
	VkDescriptorSetLayout mDescriptorSetLayout;
	VkDescriptorPool mDescriptorPool;

	VkPipelineLayout mPipelineLayout;
	VkPipeline mPipeline;

	//depth buffer
	VkImage mDepthImage;
	VkFormat mDepthImageFormat;
	VkDeviceMemory mDepthImageMemory;
	VkImageView mDepthImageView;
#pragma endregion

	//Rendering Synchronization primitives
	std::vector<VkSemaphore> mImageAvailableSemaphores;
	std::vector<VkSemaphore> mRenderFinishedSemaphores;
	std::vector<VkFence> mFrameFences;
	size_t mCurrentFrame = 0;

	VkClearValue mClearColor = { 0.0f, 0.5f, 0.5f, 1.0f };
	glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
	float mCamDist = 4.0;
	glm::vec3 mCamRotate = glm::vec3(0.0f);

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
	void createDescriptorPool(uint32_t maxSets);
	void createDescriptorSets(std::vector<VkDescriptorSet>& descriptorSets, std::shared_ptr<Texture> texture, std::vector<VkBuffer>& uniformBuffers);

	//Command Buffers
	void createFramebuffers();
	void createCommandPool();
	void createCommandBuffers();
	void createDepthBuffer();
	void createSyncObjects();




	void loadMesh(std::shared_ptr<Model> model, const std::string & meshFile);
	void createTexture(std::shared_ptr<Texture>& texture, const std::string &filename);
	void createUniformBufferObject(std::vector<VkBuffer>& uniformBuffers, std::vector<VkDeviceMemory>& uniformBuffersMemory);
	void updateUniformBuffer(std::shared_ptr<Model> model, uint32_t currentImage);

	void drawModel(VkCommandBuffer commandBuffer, std::shared_ptr<Model> model, VkDescriptorSet& descriptorSet);
};