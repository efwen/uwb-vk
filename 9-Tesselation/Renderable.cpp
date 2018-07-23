#include "Renderable.h"


Renderable::Renderable(std::shared_ptr<VulkanContext> context) :
	mContext(context)
{}

void Renderable::cleanup()
{
	vkDestroyDescriptorSetLayout(mContext->device, mDescriptorSetLayout, nullptr);

	for(auto& ubo : mUniformBuffers)
	for (size_t i = 0; i < ubo->buffers.size(); i++) {
		vkDestroyBuffer(mContext->device, ubo->buffers[i], nullptr);
		vkFreeMemory(mContext->device, ubo->buffersMemory[i], nullptr);
	}
}

void Renderable::setMesh(std::shared_ptr<Mesh> mesh)
{
	mMesh = mesh;
}

void Renderable::addTexture(std::shared_ptr<Texture> texture)
{
	mTextures.push_back(texture);
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

void Renderable::addUniformBuffer(std::shared_ptr<UBO> bufferObject)
{
	mUniformBuffers.push_back(bufferObject);
}

void Renderable::addBinding(VkDescriptorType descriptorType, VkShaderStageFlagBits stage, uint32_t bindingNum, uint32_t count)
{	
	VkDescriptorSetLayoutBinding layoutBinding = {};
	layoutBinding.descriptorType = descriptorType;
	layoutBinding.stageFlags = stage;
	layoutBinding.binding = bindingNum;
	layoutBinding.descriptorCount = count;
	layoutBinding.pImmutableSamplers = nullptr;
	mBindings.push_back(layoutBinding);
}

void Renderable::createDescriptorSetLayout()
{
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(mBindings.size());
	layoutInfo.pBindings = mBindings.data();

	if (vkCreateDescriptorSetLayout(mContext->device, &layoutInfo, nullptr, &mDescriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor set layout!");
	}
}

//probably shouldn't be in renderable
void Renderable::createDescriptorSets(const VkDescriptorPool& descriptorPool, uint32_t swapchainSize)
{
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

	for (size_t i = 0; i < swapchainSize; i++) {

		std::vector<VkDescriptorBufferInfo> buffersInfo;
		buffersInfo.resize(mUniformBuffers.size());
		for (size_t b = 0; b < buffersInfo.size(); b++)
		{
			buffersInfo[b].buffer = mUniformBuffers[b]->buffers[i];
			buffersInfo[b].offset = 0;
			buffersInfo[b].range = mUniformBuffers[b]->bufferSize;
		}

		std::vector<VkDescriptorImageInfo> texturesInfo;
		texturesInfo.resize(mTextures.size());
		for (size_t t = 0; t < texturesInfo.size(); t++)
		{
			texturesInfo[t].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			texturesInfo[t].imageView = mTextures[t]->getImageView();
			texturesInfo[t].sampler = mTextures[t]->getSampler();
		}

		std::vector <VkWriteDescriptorSet> descriptorWrites = {};
		size_t writeIndex = 0;
		uint32_t bindingNum = 0;
		for (auto& bufferInfo : buffersInfo)
		{
			VkWriteDescriptorSet bufferDescriptorWrite = {};
			bufferDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			bufferDescriptorWrite.dstSet = mDescriptorSets[i];
			bufferDescriptorWrite.dstBinding = bindingNum;
			bufferDescriptorWrite.dstArrayElement = 0;
			bufferDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			bufferDescriptorWrite.descriptorCount = 1;
			bufferDescriptorWrite.pBufferInfo = &bufferInfo;
			bufferDescriptorWrite.pImageInfo = nullptr;
			bufferDescriptorWrite.pTexelBufferView = nullptr;
			descriptorWrites.push_back(bufferDescriptorWrite);
			writeIndex++;
			bindingNum++;
		}

		for (auto& textureInfo : texturesInfo)
		{
			VkWriteDescriptorSet textureDescriptorWrite = {};
			textureDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			textureDescriptorWrite.dstSet = mDescriptorSets[i];
			textureDescriptorWrite.dstBinding = bindingNum;
			textureDescriptorWrite.dstArrayElement = 0;
			textureDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			textureDescriptorWrite.descriptorCount = 1;
			textureDescriptorWrite.pBufferInfo = nullptr;
			textureDescriptorWrite.pImageInfo = &textureInfo;
			textureDescriptorWrite.pTexelBufferView = nullptr;
			descriptorWrites.push_back(textureDescriptorWrite);
			writeIndex++;
			bindingNum++;
		}

		vkUpdateDescriptorSets(mContext->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}

		/*
		std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = mDescriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;
		descriptorWrites[0].pImageInfo = nullptr;
		descriptorWrites[0].pTexelBufferView = nullptr;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = mDescriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pBufferInfo = nullptr;
		descriptorWrites[1].pImageInfo = &textureInfo;
		descriptorWrites[1].pTexelBufferView = nullptr;

		*/
		//vkUpdateDescriptorSets(mContext->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}
