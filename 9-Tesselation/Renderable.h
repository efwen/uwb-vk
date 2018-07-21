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

	void initialize();
	void cleanup();

	//Mesh
	std::shared_ptr<Mesh> mMesh;

	//Textures
	std::shared_ptr<Texture> mTexture;

	//shaders
	ShaderSet mShaderSet;

	//UBOs
	UBO mMVPBuffer;

	//Bindings
	std::vector<VkDescriptorSetLayoutBinding> mBindings;
	VkDescriptorSetLayout mDescriptorSetLayout;
	std::vector<VkDescriptorSet> mDescriptorSets;


	//Pipeline
	VkPipelineLayout mPipelineLayout;
	VkPipeline mPipeline;

	//"Xform"
	glm::vec3 mScale = glm::vec3(1.0f);
	glm::vec3 mPosition = glm::vec3(0.0f);
	float zRotation = 0.0f;

	void setMesh(std::shared_ptr<Mesh> mesh);
	void setTexture(std::shared_ptr<Texture> texture);
	void addBinding(VkDescriptorType descriptorType, VkShaderStageFlagBits stage, uint32_t bindingNum, uint32_t count);

private:
	std::shared_ptr<VulkanContext> mContext;

};