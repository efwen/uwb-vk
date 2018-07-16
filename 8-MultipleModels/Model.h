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
#include "Texture.h"

//Model: a renderable object
struct Model
{
	//vertex buffer
	std::vector<Vertex> mVertices;
	VkBuffer mVertexBuffer;
	VkDeviceMemory mVertexBufferMemory;
	//index buffer
	std::vector<uint32_t> mIndices;
	VkBuffer mIndexBuffer;
	VkDeviceMemory mIndexBufferMemory;

	std::shared_ptr<Texture> mTexture;

	//ubo
	std::vector<VkBuffer> mUniformBuffers;
	std::vector<VkDeviceMemory> mUniformBuffersMemory;

	std::vector<VkDescriptorSet> mDescriptorSets;	//perhaps put in Model struct/class

	//"Xform"
	glm::vec3 mScale = glm::vec3(1.0f);
	glm::vec3 mPosition = glm::vec3(0.0f);
	float zRotation = 0.0f;
};