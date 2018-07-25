#include "Renderable.h"


Renderable::Renderable(std::shared_ptr<VulkanContext> context) :
	mContext(context)
{}

void Renderable::cleanup()
{
	vkDestroyDescriptorSetLayout(mContext->device, mDescriptorSetLayout, nullptr);
}

void Renderable::setMesh(std::shared_ptr<Mesh> mesh)
{
	mMesh = mesh;
}

void Renderable::bindTexture(std::shared_ptr<Texture> texture, uint32_t binding)
{
	std::cout << "Binding texture to " << binding << std::endl;
	if (mLayoutBindings.count(binding) == 0)
		throw std::runtime_error("Cannot bind Texture, descriptor set layout binding does not exist!");
	if (mLayoutBindings[binding].descriptorType != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
		throw std::runtime_error("Cannot bind texture, binding.descriptorType != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER!");

	mTextureBindings[binding] = texture;
}

void Renderable::setShader(std::shared_ptr<Shader> shader, VkShaderStageFlagBits shaderStage)
{
	if (shader->getStage() != shaderStage) {
		throw std::runtime_error("shader incompatible with target stage!");
	}

	switch (shaderStage)
	{
	case VK_SHADER_STAGE_VERTEX_BIT:
		mShaderSet.vertShader = shader;
		break;
	case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
		mShaderSet.tessControlShader = shader;
		break;
	case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
		mShaderSet.tessEvalShader = shader;
		break;
	case VK_SHADER_STAGE_GEOMETRY_BIT:
		mShaderSet.geometryShader = shader;
		break;
	case VK_SHADER_STAGE_FRAGMENT_BIT:
		mShaderSet.fragShader = shader;
		break;
	default:
		throw std::runtime_error("Cannot set shader in ShaderSet! Invalid ShaderStage parameter");
	}
}

void Renderable::applyShaderSet(ShaderSet toApply)
{
	mShaderSet = toApply;
}

void Renderable::bindUniformBuffer(std::shared_ptr<UBO> bufferObject, uint32_t binding)
{
	std::cout << "Binding ubo to " << binding << std::endl;
	if (mLayoutBindings.count(binding) == 0)
		throw std::runtime_error("Cannot bind UBO, descriptor set layout binding does not exist!");
	if (mLayoutBindings[binding].descriptorType != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		throw std::runtime_error("Cannot bind texture, binding.descriptorType != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER");

	mBufferBindings[binding] = bufferObject;
}

void Renderable::addShaderBinding(VkDescriptorType descriptorType, VkShaderStageFlagBits stage, uint32_t bindingNum, uint32_t count)
{	
	VkDescriptorSetLayoutBinding layoutBinding = {};
	layoutBinding.descriptorType = descriptorType;
	layoutBinding.stageFlags = stage;
	layoutBinding.binding = bindingNum;
	layoutBinding.descriptorCount = count;
	layoutBinding.pImmutableSamplers = nullptr;

	std::cout << "Adding binding at " << bindingNum << std::endl;
	mLayoutBindings[bindingNum] = layoutBinding;
}

void Renderable::createDescriptorSetLayout()
{
	//map is most convenient for adding & removing, but vkCreateDescriptorSetLayout
	//wants a strict array
	std::vector<VkDescriptorSetLayoutBinding> bindingsTmp;
	bindingsTmp.reserve(mLayoutBindings.size());
	for (auto const& bmap : mLayoutBindings) 
		bindingsTmp.push_back(bmap.second);

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindingsTmp.size());
	layoutInfo.pBindings = bindingsTmp.data();

	if (vkCreateDescriptorSetLayout(mContext->device, &layoutInfo, nullptr, &mDescriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor set layout!");
	}
}

void Renderable::createDescriptorSets(const VkDescriptorPool& descriptorPool, uint32_t swapchainSize)
{
	if (mBufferBindings.size() + mTextureBindings.size() != mLayoutBindings.size()) {
		throw std::runtime_error("Binding count mismatch!");
	}

	std::vector<VkDescriptorSetLayout> layouts(swapchainSize, mDescriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = swapchainSize;
	allocInfo.pSetLayouts = layouts.data();

	mDescriptorSets.resize(swapchainSize);
	VkResult result = vkAllocateDescriptorSets(mContext->device, &allocInfo, &mDescriptorSets[0]);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate descriptor set!");
	}


	for (size_t i = 0; i < swapchainSize; i++)
	{	
		std::vector <VkWriteDescriptorSet> descriptorWrites = {};
		std::vector <std::pair<uint32_t, VkDescriptorBufferInfo>> bufferInfos = {};
		std::vector <std::pair<uint32_t, VkDescriptorImageInfo>> textureInfos = {};
		
		for (const auto& binding : mBufferBindings) {
			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = binding.second->buffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = binding.second->bufferSize;

			bufferInfos.push_back(std::make_pair(binding.first, bufferInfo));
		}

		
		for (const auto& texBinding : mTextureBindings) {
			VkDescriptorImageInfo imageInfo = {};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = texBinding.second->getImageView();
			imageInfo.sampler = texBinding.second->getSampler();

			textureInfos.push_back(std::make_pair(texBinding.first, imageInfo));
		}


		//make some descriptors for the buffers
		for(const auto& info : bufferInfos){
			VkWriteDescriptorSet bufferDescriptorWrite = {};
			bufferDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			bufferDescriptorWrite.dstSet = mDescriptorSets[i];
			bufferDescriptorWrite.dstBinding = info.first;
			bufferDescriptorWrite.dstArrayElement = 0;
			bufferDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			bufferDescriptorWrite.descriptorCount = 1;
			bufferDescriptorWrite.pBufferInfo = &info.second;
			bufferDescriptorWrite.pImageInfo = nullptr;
			bufferDescriptorWrite.pTexelBufferView = nullptr;
			descriptorWrites.push_back(bufferDescriptorWrite);
		}
		
		//make descriptors for images
		for(const auto& info : textureInfos){
			VkWriteDescriptorSet textureDescriptorWrite = {};
			textureDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			textureDescriptorWrite.dstSet = mDescriptorSets[i];
			textureDescriptorWrite.dstBinding = info.first;
			textureDescriptorWrite.dstArrayElement = 0;
			textureDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			textureDescriptorWrite.descriptorCount = 1;
			textureDescriptorWrite.pBufferInfo = nullptr;
			textureDescriptorWrite.pImageInfo = &info.second;
			textureDescriptorWrite.pTexelBufferView = nullptr;
			descriptorWrites.push_back(textureDescriptorWrite);
		}

		vkUpdateDescriptorSets(mContext->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}
