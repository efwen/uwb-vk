#pragma once

//Vulkan
#include <vulkan/vulkan.h>

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>

//STL
#include <vector>
#include <memory>

//uwb-vk
#include "VulkanContext.h"
#include "Texture.h"
#include "Shader.h"
#include "Mesh.h"
#include "UBO.h"

//Model: a renderable object
class Renderable
{
public:
	Renderable(std::shared_ptr<VulkanContext> context);
	void cleanup();


	//Set Externally
	std::shared_ptr<Mesh> mMesh;
	std::vector<std::shared_ptr<Texture>> mTextures;
	ShaderSet mShaderSet;
	std::vector<VkDescriptorSetLayoutBinding> mBindings;
	std::vector <std::shared_ptr<UBO>> mUniformBuffers;

	//Descriptor Sets
	VkDescriptorSetLayout mDescriptorSetLayout;
	std::vector<VkDescriptorSet> mDescriptorSets;

	//Pipeline
	VkPipelineLayout mPipelineLayout;
	VkPipeline mPipeline;

	void setMesh(std::shared_ptr<Mesh> mesh);
	void addTexture(std::shared_ptr<Texture> texture);
	void setShader(std::shared_ptr<Shader> shader, VkShaderStageFlagBits shaderStage);
	void applyShaderSet(ShaderSet toApply);
	void addUniformBuffer(std::shared_ptr<UBO> bufferObject);
	void addBinding(VkDescriptorType descriptorType, VkShaderStageFlagBits stage, uint32_t bindingNum, uint32_t count);

	void createDescriptorSetLayout();
	void createDescriptorSets(const VkDescriptorPool& descriptorPool, uint32_t swapchainSize);
private:
	std::shared_ptr<VulkanContext> mContext;


};