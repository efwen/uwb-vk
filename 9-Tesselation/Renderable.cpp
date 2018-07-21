#include "Renderable.h"


Renderable::Renderable(std::shared_ptr<VulkanContext> context) :
	mContext(context)
{}

void Renderable::cleanup()
{
	vkDestroyDescriptorSetLayout(mContext->device, mDescriptorSetLayout, nullptr);

	for (size_t i = 0; i < mMVPBuffer.buffers.size(); i++) {
		vkDestroyBuffer(mContext->device, mMVPBuffer.buffers[i], nullptr);
		vkFreeMemory(mContext->device, mMVPBuffer.buffersMemory[i], nullptr);
	}

	//mTexture->free();
	//mMesh->free();
}

void Renderable::setMesh(std::shared_ptr<Mesh> mesh)
{
	mMesh = mesh;
}

void Renderable::setTexture(std::shared_ptr<Texture> texture)
{
	mTexture = texture;
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