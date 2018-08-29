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
#include "ShadowMap.h"


const int MAX_CONCURRENT_FRAMES = 2;	///< The number of frames in flight (2 = double buffering, etc.)
const int MAX_DESCRIPTOR_SETS = 40;		///< Maximum number of descriptor sets
const int MAX_UNIFORM_BUFFERS = 40;		///< Maximum number of UBOS
const int MAX_IMAGE_SAMPLERS = 40;		///< Maximum number of Image Samplers
const std::string SHADOW_MAP_SHADER_VERT = "Resources/Shaders/shadowPass_vert.spv";	///< Vertex Shader for the ShadowMap

/** @class RenderSystem

	@brief Primary class responsible for rendering operations.

	@author Nicholas Carpenetti

	@date 21 June 2018
*/
class RenderSystem
{
public:
	/** @brief Constructor
	*/
	RenderSystem() {}
	~RenderSystem() {}

	/** @brief Initializes the RenderSystem
	*/
	void initialize(GLFWwindow *window, const std::string& appName);
	/**
		@brief Draws a single frame
	*/
	void drawFrame();
	/**
		@brief Cleans up all allocated resources
	*/
	void cleanup();




	//--------------------------
	// Resource Creation
	//--------------------------

	/** @brief Create a Texture object

		Creates a Texture object with std::make_shared and keeps a copy
		to properly cleanup at the end
		
		@param texture			The texture object to create
		@param filename			A directory to the image file to create the texture from
	*/
	void createTexture(std::shared_ptr<Texture>& texture, const std::string &filename);

	/** @brief Create a Mesh object

		Creates a Mesh object with std::make_shared and keeps a copy
		to properly cleanup at the end
		
		@param mesh				 The mesh object to create
		@param filename			 The file to create the mesh from (must be a *.mesh file)
		@param calculateTangents Whether to calculate tangents for all of the vertices (used for normal mapping)
	*/
	void createMesh(std::shared_ptr<Mesh>& mesh, const std::string& filename, bool calculateTangents);

	/** @brief Create a Shader object

		Creates a Shader object with std::make_shared and keeps a copy
		to properly cleanup at the end

		@param shader			The shader object to create
		@param filename			The shader file to load (must a *.spv file)
		@param stage			The shader stage the shader will run in
	*/
	void createShader(std::shared_ptr<Shader>& shader, const std::string& filename, VkShaderStageFlagBits stage);

	/** @brief Create a Renderable object

		Very simple - Just creates a renderable shared_ptr, passing the
		VulkanContext in its constructor.

		@param renderable		The renderable object to create
	*/
	void createRenderable(std::shared_ptr<Renderable>& renderable);

	/** @brief Instantiate a renderable that has been set up
		
		Instaniates a renderable by creating the Descriptor Set Layouts,
		DescriptorSets, and Pipelines. Also keeps a copy of the renderable
		for cleanup, and recreates the command buffers.

		@param renderable The Renderable to instantiate
	*/
	void instantiateRenderable(std::shared_ptr<Renderable>& renderable);

	/** @brief Create a Uniform Buffer Object
	
		Note: the count parameter is untested and likely does not work,
		so leave at one.

		@param ubo The Uniform Buffer Object to create
		@param count the number of UBOs to create
	*/
	template<typename T>
	void createUniformBuffer(std::shared_ptr<UBO>& ubo, const size_t& count)
	{
		std::cout << "sizeof: " << sizeof(T) << std::endl;
		ubo = std::make_shared<UBO>();
		size_t swapchainSize = mSwapchain->size();

		ubo->bufferSize = sizeof(T);

		ubo->buffers.resize(swapchainSize * count);
		ubo->buffersMemory.resize(swapchainSize * count);
		for (size_t i = 0; i < ubo->buffers.size(); i++) {
			mBufferManager->createBuffer(ubo->bufferSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				ubo->buffers[i],
				ubo->buffersMemory[i]);
		}

		mUniformBuffers.push_back(ubo);
	}

	/** @brief Update the info inside a UBO

		@param ubo			The UBO to update
		@param uboData		The data to update the UBO with
		@param bufIndex		If the UBO is an array, this provides an index within
							the array to update at. NoteL this feature is untested
							and likely does not work. Please leave at 0.
	*/
	template<typename T>
	void updateUniformBuffer(const UBO& ubo, const T& uboData, size_t bufIndex)
	{
		//the actual index, taking into account duplicates for the swapchain
		size_t actualIndex = (ubo.buffersMemory.size() / MAX_CONCURRENT_FRAMES) * mCurrentFrame + bufIndex;

		void* data;
		vkMapMemory(mContext->device, ubo.buffersMemory[actualIndex], 0, sizeof(T), 0, &data);
		memcpy(data, &uboData, sizeof(T));
		vkUnmapMemory(mContext->device, ubo.buffersMemory[actualIndex]);
	}
	
	/** @brief Set the background clear color to a given value

		@param clearColor	The Color to set the background color as
	*/
	void setClearColor(VkClearValue clearColor);
	
	/** @brief Set the MVP buffer for a shadow-casting light source

		Set the MVP buffer for a shadow-casting light source. Currently
		This only supports a single light source, but could be modified to support
		several

		@param mvp		The mvp matrix to set.
	*/
	void setLightMVPBuffer(const glm::mat4& mvp);

	/** @brief Get the ShadowMap

		This is commonly used to set a Renderable as shadow-receiving
	*/
	ShadowMap getShadowMap() const { return mShadowMap; };

	std::vector<std::shared_ptr<Renderable>> mRenderables;	///<The renderables currently in use and being rendered
private:
	std::shared_ptr<VulkanContext> mContext;				///< The Vulkan Context object
	std::shared_ptr<CommandPool> mCommandPool;				///< The Command Pool for allocating command buffers
	std::shared_ptr<BufferManager> mBufferManager;			///< The Buffer Manager for allocating and performing buffer operations
	std::shared_ptr<ImageManager> mImageManager;			///< The Image Manager for allocation and performing Image operations

	std::unique_ptr<Swapchain> mSwapchain;					///< The Primary Swapchain Object
	std::vector<VkCommandBuffer> mCommandBuffers;			///< The Main command buffers in use (same size as the swapchain)

	std::vector<std::shared_ptr<Mesh>> mMeshes;				///< All meshes that have been created
	std::vector<std::shared_ptr<Shader>> mShaders;			///< All shader objects that have been created
	std::vector<std::shared_ptr<Texture>> mTextures;		///< All texture objects that have been created
	std::vector<std::shared_ptr<UBO>> mUniformBuffers;		///< All UBOs that have been created


	//more closely attached to a renderpass than swapchain
	std::vector<VkFramebuffer> mSwapchainFramebuffers;		///< The framebuffers the pipelines write to
	VkRenderPass mColorPass;								///< The Second, standard renderpass
	VkRenderPass mShadowRenderPass;							///< The first renderpass, creating a shadow map
	VkDescriptorPool mDescriptorPool;						///< The Descriptor Pool DescriptorSets allocate from

#pragma region DepthBuffer
	VkImage mDepthImage;									///< The image the depth buffer writes to
	VkFormat mDepthImageFormat;								///< The format of the depth buffer
	VkDeviceMemory mDepthImageMemory;						///< The memory the depth buffer is allocated from
	VkImageView mDepthImageView;							///< A VkImageView to the Depth Buffer image
#pragma endregion
	

#pragma region ShadowMapping
	ShadowMap mShadowMap;									///< A ShadowMap object for the shadow pass
	std::vector<VkFramebuffer> mShadowFramebuffers;			///< The framebuffers the shadow map's pipeline outputs to
	VkPipeline mShadowMapPipeline;							///< The pipeline the ShadowMap is processeed in
	VkPipelineLayout mShadowMapPipelineLayout;				///< The layout of the ShadowMap's pipeline
	ShaderSet mShadowMapShaderSet;							///< The Shaders used in the Shadow Pass (just one vertex shader)
	VkDescriptorSetLayout mShadowMapDescriptorSetLayout;	///< The DescriptorSetLayout for the ShadowMap pipeline
	std::vector<VkDescriptorSet> mShadowMapDescriptorSets;	///< The DescriptorSets for all the resources sent to the Shaders processing the ShadowMap
	std::shared_ptr<UBO> mShadowCasterUBO;					///< A UBO for holding the mvp matrices for the shadow-casting object
	std::vector<VkCommandBuffer> mShadowCommandBuffers;		///< Command Buffers for processing the ShadowMap
#pragma endregion

#pragma region Synchronization
	std::vector<VkSemaphore> mImageAvailableSemaphores;		///< Semaphores for indicating that a new image on the swapchain is available
	std::vector<VkSemaphore> mShadowMapAvailableSemaphores;	///< Semaphores for indicating that a new ShadowMap has been created
	std::vector<VkSemaphore> mRenderFinishedSemaphores;		///< Semaphores for indicating that a new frame has finished rendering
	std::vector<VkFence> mFrameFences;						///< Fences that ensure a frame does not start being drawn until the last frame with the same index is done.
	size_t mCurrentFrame = 0;								///< The current frame that is being drawn (index into the framebuffer array
#pragma endregion

	VkClearValue mClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };	///< The color that the screen will be cleared to
private:	
	//-----------------
	//SWAPCHAIN
	//-----------------
	
	/** @brief Create the Swapchain
	*/
	void createSwapchain();
	/** @brief Recreate the Swapchain
		
		If large changes to the RenderSystem are made (such as changing the clear color),
		the swapchain will need to be recreated with the new settings through this method
	*/
	void recreateSwapchain();
	/** @brief Cleanup Swapchain-related resources
	*/
	void cleanupSwapchain();




	//-------------
	//Pipeline
	//-------------

	/** @brief Create a new pipeline
		
		@param pipeline				The pipeline to create
		@param pipelineLayout		The layout for the pipeline, created in this method
		@param descriptorSetLayout	Descripes Which types of resources the 
										pipeline should expect as inputs for the shaders
		@param shaderStages			The shaders that the pipline will use
		@param renderPass			The renderPass the pipeline will use
	*/
	void createPipeline(VkPipeline&				pipeline, 
						VkPipelineLayout&		pipelineLayout, 
						VkDescriptorSetLayout&	descriptorSetLayout, 
						const std::vector<VkPipelineShaderStageCreateInfo>& shaderStages, 
						VkRenderPass&			renderPass);

	/** @brief Create a color renderPass object for the main pass
	*/
	void createColorRenderPass();

	/**	@brief Create framebuffers to be used by a pipeline
		@param renderPass The renderpass that will write to the framebuffers
	*/
	void createFramebuffers(VkRenderPass renderPass);





	//---------------
	//Descriptors
	//---------------

	/** @brief Create the DescriptorPool
		
		The Descriptor Pool is memory pool for allocating DescriptorSets from.

		@param maxSets				The maximum number of descriptor sets in the application
		@param maxUniformBuffers	The maximum number of UBOs that will be used
		@param maxImageSamplers		The maximum number of Image/Samplers(i.e. Textures) in the application.
	*/
	void createDescriptorPool(uint32_t maxSets, uint32_t maxUniformBuffers, uint32_t maxImageSamplers);

	/** @brief Create a DescriptorSetLayout for the Shadow pass */
	void createShadowMapDescriptorSetLayout();

	/** @brief Create the necessary Descriptor Sets to create a shadow map. */
	void createShadowMapDescriptorSets();




	//-----------------
	//Command Buffers
	//------------------

	/** @brief Create the primary command buffers for the main pass */
	void createCommandBuffers();

	/** @brief Create the primary command buffers for the shadow pass */
	void createShadowCommandBuffers();

	/** @brief Draw a renderable object

		Draw a renderable object. This method should be called during command buffer construction

		@param commandBuffer	A commandBuffer that is in the middle of recording
		@param model			A renderable object ready to be rendered
		@param descriptorSet	A descriptor set for binding the required resources for the draw command
	*/
	void drawRenderable(VkCommandBuffer commandBuffer, std::shared_ptr<Renderable> model, VkDescriptorSet& descriptorSet);

	/** @brief Create a depth buffer */
	void createDepthBuffer();

	/** @brief Create objects necessary to syncronize frame drawing 
		Sync objects ensure that each stage of the drawFrame() method operates in
		order, as calls to vkQueueSubmit and vkQueuePresentKHR are asynchronous.
		In addition they ensure that draw commands are not submitted to a frame
		that is still not done presenting.
	*/
	void createSyncObjects();




	//--------------
	//ShadowMap	
	//---------------

	/** @brief Create the ShadowMap, which is simply a depth map that can be used in the next render pass as ImageSampler input.
		The resources created as part of the ShadowMap are as follows:
			VkImage
			VkImageView
			VkSampler
	*/
	void createShadowMap();

	/** @brief Create a Pipeline for writing to the ShadowMap 
		The pipeline created is simple, in that it contains only a single vertex shader
	*/
	void createShadowMapPipeline();

	/** @brief Create a RenderPass for writing to the ShadowMap 
		@param shadowMap The ShadowMap the renderpass will write to when running
	*/
	void createShadowRenderPass(const ShadowMap& shadowMap);

	/** @brief Create the FrameBuffers used as the output
		@param shadowRenderPass The VkRenderPass describing attachments
		@param shadowImageView  The VkImageView object for the shadow map
	*/
	void createShadowFramebuffers(VkRenderPass shadowRenderPass, VkImageView shadowImageView);
};