#include "RenderSystem.h"
#include "BufferManager.h"
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <chrono>
#include <unordered_map>

void RenderSystem::initialize(GLFWwindow * window)
{
	mContext = std::make_shared<VulkanContext>(VulkanContext());
	mContext->initialize(window);

	mCommandPool = std::make_shared<CommandPool>(CommandPool(mContext));
	mCommandPool->initialize();
	
	mBufferManager = std::make_shared<BufferManager>(BufferManager(mContext, mCommandPool));

	mImageManager = std::make_shared<ImageManager>(ImageManager(mContext, mCommandPool));

	createSwapchain();
	createDescriptorPool(MAX_DESCRIPTOR_SETS, MAX_UNIFORM_BUFFERS, MAX_IMAGE_SAMPLERS);
	
	createShadowMap();
	createShadowRenderPass(mShadowMap);
	createShadowFramebuffers(mShadowRenderPass, mShadowMap.imageView);
	createShadowMapPipeline();
	createShadowCommandBuffers();
	
	//color pass creation
	createDepthBuffer();
	createColorRenderPass();					//assumes swapchain & attachments?
	createFramebuffers(mColorPass);				//needs swapchain, attachments
	createCommandBuffers();						//create a basic set of command buffers (doesn't render anything at this point)

	createSyncObjects();
}

void RenderSystem::cleanup()
{
	std::cout << "Shutting down render system" << std::endl;
	vkQueueWaitIdle(mContext->presentQueue);
	
	cleanupSwapchain();

	for (auto& model : mRenderables) {
		model->cleanup();
	}

	while (!mMeshes.empty()) {
		auto& mesh = mMeshes.back();
		mesh->free();
		mMeshes.pop_back();
	}

	while (!mTextures.empty()) {
		auto& texture = mTextures.back();
		texture->free();
		mTextures.pop_back();
	}	

	while (!mUniformBuffers.empty()) {
		auto& ubo = mUniformBuffers.back();
		for (size_t i = 0; i < mSwapchain->size(); i++) {
			vkDestroyBuffer(mContext->device, ubo->buffers[i], nullptr);
			vkFreeMemory(mContext->device, ubo->buffersMemory[i], nullptr);
		}
		mUniformBuffers.pop_back();
	}

	while (!mShaders.empty()) {
		auto& shader = mShaders.back();
		shader->free();
		mShaders.pop_back();
	}


	//cleanup descriptorpool
	vkDestroyDescriptorPool(mContext->device, mDescriptorPool, nullptr);
	
	//clean up synchronization constructs
	for (size_t i = 0; i < MAX_CONCURRENT_FRAMES; i++) {
		vkDestroySemaphore(mContext->device, mRenderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(mContext->device, mShadowMapAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(mContext->device, mImageAvailableSemaphores[i], nullptr);
		vkDestroyFence(mContext->device, mFrameFences[i], nullptr);
	}

	mCommandPool->cleanup();
	mContext->cleanup();
}

void RenderSystem::drawFrame()
{
	vkWaitForFences(mContext->device, 1, &mFrameFences[mCurrentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
	vkResetFences(mContext->device, 1, &mFrameFences[mCurrentFrame]);

	//Get the next available image
	uint32_t imageIndex;
	vkAcquireNextImageKHR(mContext->device, mSwapchain->getVkSwapchain(), std::numeric_limits<uint64_t>::max(), mImageAvailableSemaphores[mCurrentFrame], VK_NULL_HANDLE, &imageIndex);

	/*
	Pass 1: Shadows
	*/

	VkSubmitInfo shadowSubmitInfo = {};
	shadowSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	shadowSubmitInfo.pNext = nullptr;

	//semaphores to wait on last run of this frame ending
	VkSemaphore shadowWaitSemaphores[] = { mImageAvailableSemaphores[mCurrentFrame] };
	VkPipelineStageFlags shadowWaitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };	//wait for the final output of the last frame
	shadowSubmitInfo.waitSemaphoreCount = 1;
	shadowSubmitInfo.pWaitSemaphores = shadowWaitSemaphores;
	shadowSubmitInfo.pWaitDstStageMask = shadowWaitStages;
	
	//shadow command buffer to submit
	shadowSubmitInfo.commandBufferCount = 1;
	shadowSubmitInfo.pCommandBuffers = &mShadowCommandBuffers[imageIndex];

	//Semaphore to signal when done
	VkSemaphore shadowSignalSemaphores[] = { mShadowMapAvailableSemaphores[mCurrentFrame] };
	shadowSubmitInfo.signalSemaphoreCount = 1;
	shadowSubmitInfo.pSignalSemaphores = shadowSignalSemaphores;

	if (vkQueueSubmit(mContext->graphicsQueue, 1, &shadowSubmitInfo, mFrameFences[mCurrentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}


	/*
		Pass 2: Color Pass
	*/
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	//Semaphores for waiting to submit
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = shadowSignalSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	//Draw Command Buffer
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &mCommandBuffers[imageIndex];

	//Semaphore for when the submission is done
	VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphores[mCurrentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(mContext->graphicsQueue, 1, &submitInfo, /*mFrameFences[mCurrentFrame]*/ NULL) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	//Present the image
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { mSwapchain->getVkSwapchain() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &imageIndex;

	VkResult result = vkQueuePresentKHR(mContext->presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapchain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("Failed to acquire swapchain image!");
	}

	mCurrentFrame = (mCurrentFrame + 1) % MAX_CONCURRENT_FRAMES;
}

void RenderSystem::createSwapchain()
{
	mSwapchain = std::make_unique<Swapchain>(Swapchain(mContext, mImageManager));
	mSwapchain->initialize(mContext->surface, MAX_CONCURRENT_FRAMES);
}

void RenderSystem::recreateSwapchain()
{
	vkDeviceWaitIdle(mContext->device);

	cleanupSwapchain();
	createSwapchain();

	//Set up the shadowMap renderpass
	createShadowMap();
	createShadowRenderPass(mShadowMap);
	createShadowFramebuffers(mShadowRenderPass, mShadowMap.imageView);
	createShadowMapPipeline();
	createShadowCommandBuffers();

	createColorRenderPass();
	for (auto model : mRenderables) {
		createPipeline(model->mPipeline, model->mPipelineLayout, model->mDescriptorSetLayout, model->mShaderSet.createShaderInfoSet(), mColorPass);
	}
	createDepthBuffer();

	createFramebuffers(mColorPass);
	createCommandBuffers();
}

void RenderSystem::cleanupSwapchain()
{
	vkDestroyImageView(mContext->device, mDepthImageView, nullptr);
	vkDestroyImage(mContext->device, mDepthImage, nullptr);
	vkFreeMemory(mContext->device, mDepthImageMemory, nullptr);

	for (auto framebuffer : mSwapchainFramebuffers) {
		vkDestroyFramebuffer(mContext->device, framebuffer, nullptr);
	}

	mCommandPool->freeCommandBuffers(mCommandBuffers);

	for (auto renderable : mRenderables) {
		vkDestroyPipeline(mContext->device, renderable->mPipeline, nullptr);
		vkDestroyPipelineLayout(mContext->device, renderable->mPipelineLayout, nullptr);
	}
	vkDestroyRenderPass(mContext->device, mColorPass, nullptr);


	//cleanup the ShadowMap resources
	vkDestroySampler(mContext->device, mShadowMap.imageSampler, nullptr);
	vkDestroyImageView(mContext->device, mShadowMap.imageView, nullptr);
	vkDestroyImage(mContext->device, mShadowMap.image, nullptr);
	vkFreeMemory(mContext->device, mShadowMap.imageMemory, nullptr);

	for (auto framebuffer : mShadowFramebuffers) {
		vkDestroyFramebuffer(mContext->device, framebuffer, nullptr);
	}

	mCommandPool->freeCommandBuffers(mShadowCommandBuffers);

	vkDestroyPipeline(mContext->device, mShadowMapPipeline, nullptr);
	vkDestroyPipelineLayout(mContext->device, mShadowMapPipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(mContext->device, mShadowMapDescriptorSetLayout, nullptr);

	vkDestroyRenderPass(mContext->device, mShadowRenderPass, nullptr);

	mSwapchain->cleanup();
}

void RenderSystem::createPipeline(VkPipeline& pipeline, VkPipelineLayout& pipelineLayout, 
									VkDescriptorSetLayout& descriptorSetLayout, 
									const std::vector<VkPipelineShaderStageCreateInfo>& shaderStages, 
									VkRenderPass& renderPass)
{
	std::cout << "Creating Graphics pipeline" << std::endl;

	//Vertex Input:
	//Determine the format of vertex data passed into the vertex shader
	//no input for now
	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	//What kind of geometry primitives will be drawn from the vertices
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	//ViewportState
	//Where on the surface this pipeline will render to

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)mSwapchain->getExtent().width;
	viewport.height = (float)mSwapchain->getExtent().height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = mSwapchain->getExtent();

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;



	//Rasterizer
	//Turns geometry into fragments
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;


	//Multisampling - won't be using this, but useful for things like anti-aliasing
	//requires enabling a gpu feature
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	//Depth & Stencil testing
	//not using for now, so just pass in nullptr
	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;					//should the depth of new fragments be compared
	depthStencil.depthWriteEnable = VK_TRUE;				//should the depth be written?	FALSE for transparent objects?
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;		//lower depth = closer convention
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f;
	depthStencil.maxDepthBounds = 1.0f;
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {};
	depthStencil.back = {};

	//Color blending
	//For now, no blending
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VkPipelineTessellationStateCreateInfo tesselationState = {};
	tesselationState.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
	tesselationState.pNext = nullptr;
	tesselationState.flags = 0;
	tesselationState.patchControlPoints = 3;


	//Dynamic State
	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;

	//Pipeline Layout
	//This is where you pass in uniform values
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1; 
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	if (vkCreatePipelineLayout(mContext->device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}


	//Finally create the pipeline
	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.pNext = nullptr;
	pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCreateInfo.pStages = shaderStages.data();
	pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
	pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pRasterizationState = &rasterizer;
	pipelineCreateInfo.pMultisampleState = &multisampling;
	pipelineCreateInfo.pDepthStencilState = &depthStencil;
	pipelineCreateInfo.pColorBlendState = &colorBlending;
	pipelineCreateInfo.pDynamicState = nullptr;
	pipelineCreateInfo.pTessellationState = &tesselationState;

	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.renderPass = renderPass;
	pipelineCreateInfo.subpass = 0;

	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCreateInfo.basePipelineIndex = -1;

	if (vkCreateGraphicsPipelines(mContext->device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a graphics pipeline!");
	}
}

void RenderSystem::createShadowMapPipeline()
{
	mShadowMapShaderSet.vertShader = std::make_shared<Shader>(Shader(mContext));
	createShader(mShadowMapShaderSet.vertShader, SHADOW_MAP_SHADER_VERT, VK_SHADER_STAGE_VERTEX_BIT);

	createShadowMapDescriptorSetLayout();	//just one light for now
	

	//create the UBOs for shadow mapping
	createUniformBuffer<glm::mat4>(mShadowCasterUBO, 1);

	createShadowMapDescriptorSets();
	
	createPipeline(mShadowMapPipeline, 
					mShadowMapPipelineLayout, 
					mShadowMapDescriptorSetLayout, 
					mShadowMapShaderSet.createShaderInfoSet(), 
					mShadowRenderPass);
}

void RenderSystem::createColorRenderPass()
{
	//color attachment
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = mSwapchain->getImageFormat();
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;	//clear values to constant at start
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;	//rendered contents stored in memory for later
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	
	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


	//depth buffer
	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = mDepthImageFormat;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;		//clear to constant at start
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;	//not used after drawing finished
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	//not necessary right now, but will come into play w/ multipass rendering
	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;


	if (vkCreateRenderPass(mContext->device, &renderPassInfo, nullptr, &mColorPass) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create render pass!");
	}
}

void RenderSystem::createShadowRenderPass(const ShadowMap& shadowMap)
{
	VkAttachmentDescription shadowAttachment = {};
	shadowAttachment.format = shadowMap.imageFormat;
	shadowAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	shadowAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;					//clear to constant at start
	shadowAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;				//store for use in the next pass
	shadowAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	shadowAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	shadowAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	shadowAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	shadowAttachment.flags = 0;

	VkAttachmentReference shadowAttachmentRef = {};
	shadowAttachmentRef.attachment = 0;
	shadowAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.flags = 0;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = nullptr;
	subpass.colorAttachmentCount = 0;
	subpass.pColorAttachments = nullptr;
	subpass.pResolveAttachments = nullptr;
	subpass.pDepthStencilAttachment = &shadowAttachmentRef;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = nullptr;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pNext = nullptr;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &shadowAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 0;
	renderPassInfo.pDependencies = nullptr;
	renderPassInfo.flags = 0;

	if (vkCreateRenderPass(mContext->device, &renderPassInfo, nullptr, &mShadowRenderPass) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create shadow render pass!");
	}
}

void RenderSystem::createDescriptorPool(uint32_t maxSets, uint32_t maxUniformBuffers, uint32_t maxImageSamplers)
{
	assert(mSwapchain != nullptr);

	std::array<VkDescriptorPoolSize, 2> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;			//#1: MVP matrices
	poolSizes[0].descriptorCount = maxUniformBuffers;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;	//#2: image + sampler
	poolSizes[1].descriptorCount = maxImageSamplers;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = mSwapchain->size() * maxSets;

	if (vkCreateDescriptorPool(mContext->device, &poolInfo, nullptr, &mDescriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor pool!");
	}
}

void RenderSystem::createShadowMapDescriptorSetLayout()
{
	//Set up a descriptorsetlayout/descriptorset for the shadowMap
	VkDescriptorSetLayoutBinding layoutBinding = {};
	layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;		//we're taking in a UBO w/ a MVP matrix
	layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBinding.binding = 0;
	layoutBinding.descriptorCount = 1;
	layoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &layoutBinding;

	if (vkCreateDescriptorSetLayout(mContext->device, &layoutInfo, nullptr, &mShadowMapDescriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor set layout!");
	}
}

void RenderSystem::createShadowMapDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(mSwapchain->size(), mShadowMapDescriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = mDescriptorPool;
	allocInfo.descriptorSetCount = mSwapchain->size();
	allocInfo.pSetLayouts = layouts.data();

	mShadowMapDescriptorSets.resize(mSwapchain->size());
	VkResult result = vkAllocateDescriptorSets(mContext->device, &allocInfo, &mShadowMapDescriptorSets[0]);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate shadow map descriptor set!");
	}


	//assign a UBO to this descriptor (just one for now)
	for (size_t i = 0; i < mSwapchain->size(); i++) {
		VkDescriptorBufferInfo bufferInfo;
		bufferInfo.buffer = mShadowCasterUBO->buffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = mShadowCasterUBO->bufferSize;

		VkWriteDescriptorSet bufferDescriptorWrite = {};
		bufferDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		bufferDescriptorWrite.dstSet = mShadowMapDescriptorSets[i];
		bufferDescriptorWrite.dstBinding = 0;									//bind to location 0
		bufferDescriptorWrite.dstArrayElement = 0;
		bufferDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bufferDescriptorWrite.descriptorCount = 1;
		bufferDescriptorWrite.pBufferInfo = &bufferInfo;
		bufferDescriptorWrite.pImageInfo = nullptr;
		bufferDescriptorWrite.pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(mContext->device, 1, &bufferDescriptorWrite, 0, nullptr);
	}
}

void RenderSystem::createFramebuffers(VkRenderPass renderPass)
{
	std::vector<VkImageView> swapchainImageViews = mSwapchain->getImageViews();

	mSwapchainFramebuffers.resize(swapchainImageViews.size());
	for (size_t i = 0; i < swapchainImageViews.size(); i++) {
		std::array<VkImageView, 2> attachments = {
			swapchainImageViews[i],
			mDepthImageView
		};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.pNext = nullptr;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = mSwapchain->getExtent().width;
		framebufferInfo.height = mSwapchain->getExtent().height;
		framebufferInfo.layers = 1;
		framebufferInfo.flags = 0;

		if (vkCreateFramebuffer(mContext->device, &framebufferInfo, nullptr, &mSwapchainFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void RenderSystem::createShadowFramebuffers(VkRenderPass shadowRenderPass, VkImageView shadowImageView)
{
	std::cout << "Creating Shadow Framebuffer" << std::endl;

	mShadowFramebuffers.resize(mSwapchain->size());
	for (size_t i = 0; i < mShadowFramebuffers.size(); i++)
	{
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.pNext = nullptr;
		framebufferInfo.renderPass = shadowRenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &shadowImageView;
		framebufferInfo.width = mSwapchain->getExtent().width;
		framebufferInfo.height = mSwapchain->getExtent().height;
		framebufferInfo.layers = 1;
		framebufferInfo.flags = 0;

		if (vkCreateFramebuffer(mContext->device, &framebufferInfo, nullptr, &mShadowFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create shadow framebuffer!");
		}
	}
}

void RenderSystem::setLightMVPBuffer(const glm::mat4 & mvp)
{
	updateUniformBuffer<glm::mat4>(*mShadowCasterUBO, mvp, 0);
}

void RenderSystem::createCommandBuffers()
{
	std::cout << "Creating command buffers" << std::endl;

	mCommandBuffers.resize(mSwapchainFramebuffers.size());
	mCommandPool->allocateCommandBuffers(mCommandBuffers, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	//record into the command buffers
	for (size_t i = 0; i < mCommandBuffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		if (vkBeginCommandBuffer(mCommandBuffers[i], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo colorPassInfo = {};
		colorPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		colorPassInfo.pNext = nullptr;
		colorPassInfo.renderPass = mColorPass;
		colorPassInfo.framebuffer = mSwapchainFramebuffers[i];
		colorPassInfo.renderArea.offset = { 0, 0 };
		colorPassInfo.renderArea.extent = mSwapchain->getExtent();
		
		//set up clear values as part of renderPassInfo
		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = mClearColor.color;
		clearValues[1].depthStencil = { 1.0f, 0 };

		colorPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		colorPassInfo.pClearValues = clearValues.data();

		//begin the render pass
		vkCmdBeginRenderPass(mCommandBuffers[i], &colorPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		for (auto& renderable : mRenderables) {
			vkCmdBindPipeline(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, renderable->mPipeline);
			drawRenderable(mCommandBuffers[i], renderable, renderable->mDescriptorSets[i]);
		}

		vkCmdEndRenderPass(mCommandBuffers[i]);

		if (vkEndCommandBuffer(mCommandBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}

void RenderSystem::createShadowCommandBuffers()
{
	mShadowCommandBuffers.resize(mShadowFramebuffers.size());
	mCommandPool->allocateCommandBuffers(mShadowCommandBuffers, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	//record into the command buffers
	for (size_t i = 0; i < mShadowCommandBuffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		if (vkBeginCommandBuffer(mShadowCommandBuffers[i], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording shadow command buffer!");
		}

		/*
		Shadow Pass
		*/
		VkClearValue shadowClearValues[1];
		shadowClearValues[0].depthStencil.depth = 1.0f;
		shadowClearValues[0].depthStencil.stencil = 0;

		VkRenderPassBeginInfo shadowPassInfo = {};
		shadowPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		shadowPassInfo.pNext = nullptr;
		shadowPassInfo.renderPass = mShadowRenderPass;
		shadowPassInfo.framebuffer = mShadowFramebuffers[i];
		shadowPassInfo.renderArea.offset = { 0, 0 };
		shadowPassInfo.renderArea.extent = mSwapchain->getExtent();		//may be too big
		shadowPassInfo.clearValueCount = 1;
		shadowPassInfo.pClearValues = shadowClearValues;

		vkCmdBeginRenderPass(mShadowCommandBuffers[i], &shadowPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		/*
		VkViewport viewport;
		viewport.height = SHADOW_MAP_HEIGHT;
		viewport.width = SHADOW_MAP_WIDTH;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		viewport.x = 0;
		viewport.y = 0;
		vkCmdSetViewport(shadow_map_cmd_buf, 0, 1, &viewport);

		VkRect2D scissor;
		scissor.extent.width = SHADOW_MAP_WIDTH;
		scissor.extent.height = SHADOW_MAP_HEIGHT;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		vkCmdSetScissor(shadow_map_cmd_buf, 0, 1, &scissor);
		*/

		vkCmdBindPipeline(mShadowCommandBuffers[i],
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			mShadowMapPipeline);

		for (auto& renderable : mRenderables) {
			VkBuffer vertexBuffers[1] = { renderable->mMesh->getVertexBuffer() };
			VkDeviceSize offsets[] = { 0 };

			vkCmdBindVertexBuffers(mShadowCommandBuffers[i], 0, 1, vertexBuffers, offsets);

			vkCmdBindIndexBuffer(mShadowCommandBuffers[i],
				renderable->mMesh->getIndexBuffer(),
				0, VK_INDEX_TYPE_UINT32);

			vkCmdBindDescriptorSets(mShadowCommandBuffers[i],
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				mShadowMapPipelineLayout,
				0, 1,
				&mShadowMapDescriptorSets[i],
				0, nullptr);

			//Draw our model
			vkCmdDrawIndexed(mShadowCommandBuffers[i], renderable->mMesh->getIndexCount(), 1, 0, 0, 0);
		}

		vkCmdEndRenderPass(mShadowCommandBuffers[i]);

		if (vkEndCommandBuffer(mShadowCommandBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record shadow command buffer!");
		}
	}
}

void RenderSystem::drawRenderable(VkCommandBuffer commandBuffer, std::shared_ptr<Renderable> model, VkDescriptorSet& descriptorSet)
{
	//Set up draw info
	VkBuffer vertexBuffers[1] = { model->mMesh->getVertexBuffer()};
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, model->mMesh->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, model->mPipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

	//Draw our model
	vkCmdDrawIndexed(commandBuffer, model->mMesh->getIndexCount(), 1, 0, 0, 0);
}

void RenderSystem::createSyncObjects()
{
	std::cout << "Creating semaphores" << std::endl;
	mImageAvailableSemaphores.resize(MAX_CONCURRENT_FRAMES);
	mShadowMapAvailableSemaphores.resize(MAX_CONCURRENT_FRAMES);
	mRenderFinishedSemaphores.resize(MAX_CONCURRENT_FRAMES);
	mFrameFences.resize(MAX_CONCURRENT_FRAMES);
	
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_CONCURRENT_FRAMES; i++)
	{
		if(vkCreateSemaphore(mContext->device, &semaphoreInfo, nullptr, &mImageAvailableSemaphores[i]) != VK_SUCCESS ||
		   vkCreateSemaphore(mContext->device, &semaphoreInfo, nullptr, &mShadowMapAvailableSemaphores[i]) != VK_SUCCESS ||
		   vkCreateSemaphore(mContext->device, &semaphoreInfo, nullptr, &mRenderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(mContext->device, &fenceInfo, nullptr, &mFrameFences[i]) != VK_SUCCESS) 
		{
			throw std::runtime_error("Failed to create frame sync objects!");
		}
	}
}



void RenderSystem::createRenderable(std::shared_ptr<Renderable>& renderable)
{
	renderable = std::make_shared<Renderable>(Renderable(mContext));
}

void RenderSystem::instantiateRenderable(std::shared_ptr<Renderable>& renderable)
{
	renderable->createDescriptorSetLayout();
	renderable->createDescriptorSets(mDescriptorPool, mSwapchain->size());


	createPipeline(renderable->mPipeline, renderable->mPipelineLayout,
		renderable->mDescriptorSetLayout,
		renderable->mShaderSet.createShaderInfoSet(),
		mColorPass);

	mRenderables.push_back(renderable);

	//recreate the command buffers to include the new model
	createShadowCommandBuffers();
	createCommandBuffers();
}

void RenderSystem::createTexture(std::shared_ptr<Texture>& texture, const std::string &filename)
{
	std::cout << "Creating texture \"" << filename << "\"" << std::endl;
	int width, height, channels;
	stbi_uc* pixels = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);

	if (!pixels) {
		throw std::runtime_error("Failed to load texture image");
	}

	texture = std::make_shared<Texture>(Texture(mContext, mBufferManager, mImageManager));
	texture->load(pixels, width, height, channels);
	stbi_image_free(pixels);

	mTextures.push_back(texture);
}

void RenderSystem::createMesh(std::shared_ptr<Mesh>& mesh, const std::string & filename, bool calculateTangents)
{
	std::cout << "creating mesh \"" << filename << "\"" << std::endl;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	readObjFile(filename, vertices, indices, VK_FRONT_FACE_CLOCKWISE);

	//if needed, calculate tangents of the vertices
	//this is used for normal mapping
	if (calculateTangents)
	{
		for (int i = 0; i < indices.size(); i += 3)
		{
			uint32_t triIdx1 = indices[i];
			uint32_t triIdx2 = indices[i + 1];
			uint32_t triIdx3 = indices[i + 2];

			glm::vec3 edge1 = vertices[triIdx2].pos - vertices[triIdx1].pos;
			glm::vec3 edge2 = vertices[triIdx3].pos - vertices[triIdx1].pos;
			glm::vec2 deltaUV1 = vertices[triIdx2].texCoord - vertices[triIdx1].texCoord;
			glm::vec2 deltaUV2 = vertices[triIdx3].texCoord - vertices[triIdx1].texCoord;


			float fractional = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

			glm::vec3 tangent;

			tangent.x = fractional * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
			tangent.y = fractional * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
			tangent.z = fractional * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
			tangent = glm::normalize(tangent);

			vertices[triIdx1].tangent = tangent;
			vertices[triIdx2].tangent = tangent;
			vertices[triIdx3].tangent = tangent;
		}
	}


	mesh = std::make_shared<Mesh>(Mesh(mContext, mBufferManager));
	mesh->load(vertices, indices);

	mMeshes.push_back(mesh);
}

void RenderSystem::createShader(std::shared_ptr<Shader>& shader, const std::string & filename, VkShaderStageFlagBits stage)
{
	std::cout << "creating shader \"" << filename << "\"" << std::endl;
	std::vector<char> code = readShaderFile(filename);

	shader = std::make_shared<Shader>(Shader(mContext));
	shader->load(code, stage);

	mShaders.push_back(shader);
}

void RenderSystem::createDepthBuffer()
{
	std::cout << "Creating depth resources" << std::endl;

	std::cout << "Finding a depth image format" << std::endl;
	mDepthImageFormat = mImageManager->findSupportedFormat(
						{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
						VK_IMAGE_TILING_OPTIMAL,
						VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	
	//create the depth image
	std::cout << "creating depth image" << std::endl;
	mImageManager->createImage(mSwapchain->getExtent().width,
						mSwapchain->getExtent().height,
						mDepthImageFormat,
						VK_IMAGE_TILING_OPTIMAL,
						VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
						VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
						mDepthImage,
						mDepthImageMemory);

	//make an image view so we know how to access the depth image
	std::cout << "creating depth image view" << std::endl;
	mDepthImageView = mImageManager->createImageView(mDepthImage, mDepthImageFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

	//transition to a layout suitable for depth attachment use
	std::cout << "transitioning from undefinied to depth_stencil_attachment_optimal" << std::endl;
	mImageManager->transitionImageLayout(mDepthImage,
									mDepthImageFormat,
									VK_IMAGE_LAYOUT_UNDEFINED,
									VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	//make the sampler

}

void RenderSystem::createShadowMap()
{
	std::cout << "Creating Shadow Map resources" << std::endl;
	mShadowMap.imageFormat = VK_FORMAT_D32_SFLOAT;

	std::cout << "Creating shadow image" << std::endl;
	mImageManager->createImage(mSwapchain->getExtent().width,
		mSwapchain->getExtent().height,
		mShadowMap.imageFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
		VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		mShadowMap.image,
		mShadowMap.imageMemory);


	std::cout << "Creatring shadow imageview" << std::endl;
	mShadowMap.imageView = mImageManager->createImageView(mShadowMap.image, mShadowMap.imageFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

	//create a sampler so we can access it in other shaders
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

	//anisotropic filtering
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = 1.0f;

	//what color to return when sampling beyond image in clamp address mode
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

	//use texel values from [0, 1)
	samplerInfo.unnormalizedCoordinates = VK_FALSE;

	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 100.0f;

	if (vkCreateSampler(mContext->device, &samplerInfo, nullptr, &mShadowMap.imageSampler) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create texture sampler!");
	}
}

void RenderSystem::setClearColor(VkClearValue clearColor)
{
	//wait for the device to be idle, then recreate the command buffers with the new clear color
	vkDeviceWaitIdle(mContext->device);
	mClearColor = clearColor;
	createShadowCommandBuffers();
	createCommandBuffers();
}