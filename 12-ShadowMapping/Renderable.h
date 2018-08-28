#pragma once

//Vulkan
#include <vulkan/vulkan.h>

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>

//STL
#include <vector>
#include <memory>
#include <map>

//uwb-vk
#include "VulkanContext.h"
#include "Texture.h"
#include "Shader.h"
#include "Mesh.h"
#include "UBO.h"
#include "ShadowMap.h"

/**	@class Renderable
	@brief A Class for representing objects that are rendered in the scene

	This class Represents objects that are rendered within a scene in the
	RenderSystem. It consists of: a mesh, a shaderSet, Bindings,
	A descriptorSetLayout, descriptorSets, a pipeline layout, and a pipeline.
*/
class Renderable
{
public:
	/** @brief Constructor
		@param context The Render System's Vulkan Context object
	*/
	Renderable(std::shared_ptr<VulkanContext> context);

	/** @brief clean up resources allocated by the Renderable
	*/
	void cleanup();



	//-----------------
	// Setup Methods
	//-----------------

	/** @brief Set the mesh used by this Renderable
		@param mesh The mesh This Renderable will use
	*/
	void setMesh(std::shared_ptr<Mesh> mesh);

	/** @brief Set a specific Shader to be used by this Renderable
		@param shader The shader being applied
		@param shaderStage The pipeline stage the shader will be used at
	*/
	void setShader(std::shared_ptr<Shader> shader, VkShaderStageFlagBits shaderStage);

	/** @brief Apply an entire ShaderSet to this renderable
		@param toApply The Shader taht is going to be applied
	*/
	void applyShaderSet(const ShaderSet& toApply);




	//------------------------
	// Bindings Setup
	//------------------------

	/** @brief Note a specific binding expected by the shaders
		
		This method does not actually bind resources but prepares a list of 
		what bindings to expect. When actually binding resources, this list
		is checked to ensure that all the expected resources are filled and
		there are no duplicates, etc.
		
		@param descriptorType	The type of descriptor to use (i.e image/sampler or uniform buffer)
		@param stage			The stage the binding is at
		@param bindingNum		The number to bind at
		@param count			The number of resources of that type to bind to the given bindingNum
	*/
	void addShaderBinding(VkDescriptorType descriptorType, VkShaderStageFlagBits stage, uint32_t bindingNum, uint32_t count);

	/** @brief Bind a texture to be used by the renderable
		
		@param texture			The texture to bind
		@param binding			The value to bind the texture
	*/
	void bindTexture(std::shared_ptr<Texture> texture, uint32_t binding);

	/** @brief Bind a Uniform Buffer to be used by the renderable
	
		@param bufferObject		The UBO to be bound to the pipeline
		@param binding			The value at which to bind the UBO
	*/
	void bindUniformBuffer(std::shared_ptr<UBO> bufferObject, uint32_t binding);

	/** @brief Bind a ShadowMap to be used by the renderable
		
		@param shadowMap		The shadowMap to bind
		@param binding			The value at which to bind the ShadowMap
	*/
	void bindShadowMap(const ShadowMap& shadowMap, uint32_t binding);



	//--------------------------
	// Descriptor Set Setup
	//---------------------------
	
	/** @brief Create the VkDescriptorSetLayout this Renderable uses
	*/
	void createDescriptorSetLayout();

	/** @brief Create and write the VkDescriptorSets that will by used by this Renderable
		
		This method checks against the shader Bindings for missing resources. Then,
		it creates a new Descriptor set for every swapchain image that contains all of the
		bindings used by this Renderable

		@param descriptorPool	The descriptorPool to draw from when making new descriptor sets
		@param swapchainSize	The number of images in the swapchain
	*/
	void createDescriptorSets(const VkDescriptorPool& descriptorPool, uint32_t swapchainSize);
public:
	std::shared_ptr<Mesh> mMesh;										///< The Mesh used by this Renderable
	ShaderSet mShaderSet;												///< The set of Shaders used by this Renderable

	std::map<uint32_t, VkDescriptorSetLayoutBinding> mLayoutBindings;	///< All of the bindings used by this Renderable
	std::map<uint32_t, std::shared_ptr<UBO>> mBufferBindings;			///< The UBOs that are bound to this Renderable
	std::map<uint32_t, std::shared_ptr<Texture>> mTextureBindings;		///< The Textures that are bound to this Renderable
	std::map<uint32_t, ShadowMap> mShadowMapBindings;					///< The ShadowMaps that are bound to this Renderable


	VkDescriptorSetLayout mDescriptorSetLayout;							///< The descriptorSetLayout Used by this Renderable
	std::vector<VkDescriptorSet> mDescriptorSets;						///< DescriptorSets used by this Renderable


	VkPipelineLayout mPipelineLayout;									///< The layout of the pipeline used by this Renderable
	VkPipeline mPipeline;												///< The pipeliner used by this Renderable
private:
	std::shared_ptr<VulkanContext> mContext;							///< The Render System's Vulkan Context
};