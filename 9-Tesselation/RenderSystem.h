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
#include "Extensions.h"
#include "VulkanContext.h"
#include "QueueFamilies.h"
#include "CommandPool.h"
#include "BufferManager.h"
#include "ImageManager.h"
#include "Swapchain.h"
#include "Texture.h"
#include "Vertex.h"
#include "Renderable.h"
#include "Shader.h"
#include "Mesh.h"


struct MVPMatrices {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

const int MAX_CONCURRENT_FRAMES = 2;
const int DESCRIPTOR_POOL_SIZE = 4;

class RenderSystem
{
public:
	RenderSystem() {}
	~RenderSystem() {}

	void initialize(GLFWwindow *window);
	void drawFrame();
	void cleanup();

	//create a Model to render (probably better to use Mesh + Texture objects for reuse
	void createTexture(std::shared_ptr<Texture>& texture, const std::string &filename);
	void createMesh(std::shared_ptr<Mesh>& mesh, const std::string& filename);
	void createShader(std::shared_ptr<Shader>& shader, const std::string& filename, VkShaderStageFlagBits stage);
	void createRenderable(std::shared_ptr<Renderable>& renderable);
	void instantiateRenderable(std::shared_ptr<Renderable>& renderable);

	template<typename T>
	void createUniformBuffer(std::shared_ptr<UBO>& ubo)
	{
		ubo = std::make_shared<UBO>(UBO());
		size_t swapchainSize = mSwapchain->size();

		ubo->bufferSize = sizeof(T);
		ubo->buffers.resize(swapchainSize);
		ubo->buffersMemory.resize(swapchainSize);

		for (size_t i = 0; i < swapchainSize; i++) {
			mBufferManager->createBuffer(ubo->bufferSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				ubo->buffers[i],
				ubo->buffersMemory[i]);
		}
	}

	template<typename T>
	void updateUniformBuffer(UBO& ubo, T& uboData)
	{
		void* data;
		vkMapMemory(mContext->device, ubo.buffersMemory[mCurrentFrame], 0, sizeof(uboData), 0, &data);
		memcpy(data, &uboData, sizeof(T));
		vkUnmapMemory(mContext->device, ubo.buffersMemory[mCurrentFrame]);
	}
	
	void setClearColor(VkClearValue clearColor);

	std::vector<std::shared_ptr<Renderable>> mRenderables;
private:
	std::shared_ptr<VulkanContext> mContext;
	std::shared_ptr<CommandPool> mCommandPool;
	std::shared_ptr<BufferManager> mBufferManager;
	std::shared_ptr<ImageManager> mImageManager;

	std::unique_ptr<Swapchain> mSwapchain;
	std::vector<VkCommandBuffer> mCommandBuffers;

	//some defaults so that we (hopefully) never have an invalid model
	std::shared_ptr<Mesh> mDefaultMesh;				//when no mesh loaded
	std::shared_ptr<ShaderSet> mDefaultShaderSet;	//when no shader set loaded
	std::shared_ptr<Texture> mDefaultTexture;		//when no texture loaded

	std::vector<std::shared_ptr<Mesh>> mMeshes;
	std::vector<std::shared_ptr<Shader>> mShaders;
	std::vector<std::shared_ptr<Texture>> mTextures;
	std::vector<std::shared_ptr<UBO>> mUniformBuffers;


	//more closely attached to a renderpass than swapchain
	std::vector<VkFramebuffer> mSwapchainFramebuffers;
	VkRenderPass mRenderPass;
	VkDescriptorPool mDescriptorPool;
#pragma endregion

#pragma region DepthBuffer
	//depth buffer
	VkImage mDepthImage;
	VkFormat mDepthImageFormat;
	VkDeviceMemory mDepthImageMemory;
	VkImageView mDepthImageView;
#pragma endregion

#pragma region Synchronization
	std::vector<VkSemaphore> mImageAvailableSemaphores;
	std::vector<VkSemaphore> mRenderFinishedSemaphores;
	std::vector<VkFence> mFrameFences;
	size_t mCurrentFrame = 0;
#pragma endregion

	VkClearValue mClearColor = { 0.0f, 0.5f, 0.5f, 1.0f };
private:	
	//SWAPCHAIN
	void createSwapchain();
	void recreateSwapchain();
	void cleanupSwapchain();

	//Pipeline
	void createPipeline(VkPipeline& pipeline, VkPipelineLayout& pipelineLayout, VkDescriptorSetLayout& descriptorSetLayout, const std::vector<VkPipelineShaderStageCreateInfo>& shaderStages, VkRenderPass& renderPass);
	void createRenderPass();
	void createFramebuffers(VkRenderPass renderPass);

	//Descriptors
	void createDescriptorPool(uint32_t maxSets);
	void createDescriptorSetLayout(VkDescriptorSetLayout& layout, std::vector<VkDescriptorSetLayoutBinding>& binding);
	void createDescriptorSets(VkDescriptorSetLayout& layout, std::vector<VkDescriptorSet>& descriptorSets, std::shared_ptr<Texture> texture, UBO& mvpBuffer);

	//Command Buffers
	void createCommandBuffers();
	void drawRenderable(VkCommandBuffer commandBuffer, std::shared_ptr<Renderable> model, VkDescriptorSet& descriptorSet);

	void createDepthBuffer();
	void createSyncObjects();
};