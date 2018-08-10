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
	assert(bufferObject != nullptr);

	std::cout << "Binding ubo to " << binding << std::endl;
	if (mLayoutBindings.count(binding) == 0)
		throw std::runtime_error("Cannot bind UBO, descriptor set layout binding does not exist!");
	if (mLayoutBindings[binding].descriptorType != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		throw std::runtime_error("Cannot bind texture, binding.descriptorType != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER");

	mBufferBindings[binding] = bufferObject;
}

//Add a binding a particular shader is expecting to an std::map
//This map will be referenced when binding resources to the renderable
//	to check if the binding make sense
//Ideally this would be done through a reflection process
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
		using BufferInfoSet = std::pair<uint32_t, std::vector<VkDescriptorBufferInfo>>;		//each binding can have multiple infos associated (as in an array of buffers)
		using ImageInfoSet = std::pair<uint32_t, std::vector<VkDescriptorImageInfo>>;

		std::vector <BufferInfoSet> bufferInfos = {};
		std::vector <ImageInfoSet> textureInfos = {};
		
		for (const auto& bufBinding : mBufferBindings) {

			BufferInfoSet infoSet;
			infoSet.first = bufBinding.first;

			uint32_t descCount = mLayoutBindings[bufBinding.first].descriptorCount;
			for (uint32_t bufIdx = 0; bufIdx < descCount; bufIdx++)
			{
				VkDescriptorBufferInfo bufferInfo = {};
				bufferInfo.buffer = bufBinding.second->buffers[descCount * i + bufIdx];
				bufferInfo.offset = 0;
				bufferInfo.range = bufBinding.second->bufferSize;
				infoSet.second.push_back(bufferInfo);
			}
			bufferInfos.push_back(infoSet);
		}
		
		for (const auto& texBinding : mTextureBindings) {

			ImageInfoSet infoSet;
			infoSet.first = texBinding.first;
			for (uint32_t texIdx = 0; texIdx < mLayoutBindings[texBinding.first].descriptorCount; texIdx++)
			{
				VkDescriptorImageInfo imageInfo = {};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo.imageView = texBinding.second->getImageView();
				imageInfo.sampler = texBinding.second->getSampler();
				infoSet.second.push_back(imageInfo);
			}

			textureInfos.push_back(infoSet);
		}


		//make some descriptors for the buffers
		for(const auto& info : bufferInfos){
			VkWriteDescriptorSet bufferDescriptorWrite = {};
			bufferDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			bufferDescriptorWrite.dstSet = mDescriptorSets[i];
			bufferDescriptorWrite.dstBinding = info.first;
			bufferDescriptorWrite.dstArrayElement = 0;
			bufferDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			bufferDescriptorWrite.descriptorCount = mLayoutBindings[info.first].descriptorCount;
			bufferDescriptorWrite.pBufferInfo = info.second.data();
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
			textureDescriptorWrite.descriptorCount = mLayoutBindings[info.first].descriptorCount;
			textureDescriptorWrite.pBufferInfo = nullptr;
			textureDescriptorWrite.pImageInfo = info.second.data();
			textureDescriptorWrite.pTexelBufferView = nullptr;
			descriptorWrites.push_back(textureDescriptorWrite);
		}

		vkUpdateDescriptorSets(mContext->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}
