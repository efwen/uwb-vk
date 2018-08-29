#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>
#include <memory>

#include "VulkanContext.h"

/** @class Shader
	
	@brief A Single Shader module and shader stage create info

	@author Nicholas Carpenetti

	@date 20 July 2018
*/
class Shader
{
public:
	/** @brief Constructor
		@param context The RenderSystem's VulkanContext object
	*/
	Shader(std::shared_ptr<VulkanContext> context);
	~Shader();

	/** @brief Create a shader object from SPIR-V code, and set a stage during which it will be used
		This method immediately creates a VkShaderModule object, and discards the code given
		@param code A byte array of the loaded SPIR-V code
		@param stage The stage during which this shader will be run in a pipeline
	*/
	void load(const std::vector<char>& code, VkShaderStageFlagBits stage);
	
	/** @brief Free the Vulkan resources created by the Shader Object */
	void free();

	//---------
	// Accessors
	//---------

	/** @brief Retrieve a VkPipelineShaderStageCreateInfo object from the shader Information */
	VkPipelineShaderStageCreateInfo getShaderStageInfo() const;
	
	/** @brief Get the stage this shader is used in (Vertex, Fragment, etc.) */
	VkShaderStageFlagBits getStage() const;
protected:
	std::shared_ptr<VulkanContext> mContext;				///< The Vulkan Context Object

	VkShaderStageFlagBits mStage = VK_SHADER_STAGE_ALL;		///< The stage the shader operates during
	VkShaderModule mShaderModule = VK_NULL_HANDLE;			///< The shader module (created in load())
	VkPipelineShaderStageCreateInfo mShaderStageInfo;		///< Info for creating a shader stage in pipeline creation

	/** @brief Create the VkShaderModule object
		@param code A byte array containing the SPIR-V shader code
	*/
	void createShaderModule(const std::vector<char>& code);
};



/** @class ShaderSet

	@brief A Set of shaders to be used in a pipeline

	Contains a pointer for each programmable stage of the pipeline. A value of
	null indicates that that stage is not used. This struct is used insid

	@author Nicholas Carpenetti

	@date 20 July 2018
*/
struct ShaderSet {
	std::shared_ptr<Shader> vertShader = nullptr;			///< Vertex Shader
	std::shared_ptr<Shader> tessControlShader = nullptr;	///< Tessellation Control Shader
	std::shared_ptr<Shader> tessEvalShader = nullptr;		///< Tessellation Evaluation Shader
	std::shared_ptr<Shader> geometryShader = nullptr;		///< Geometry Shader
	std::shared_ptr<Shader> fragShader = nullptr;			///< Fragment Shader

	/** @brief Creates a VkShaderInfoSet object based on the shaders used in this set */
	std::vector<VkPipelineShaderStageCreateInfo> createShaderInfoSet()
	{
		std::vector<VkPipelineShaderStageCreateInfo> shaderInfoSet;

		if (vertShader) 
			shaderInfoSet.push_back(vertShader->getShaderStageInfo());
		if (tessControlShader) 
			shaderInfoSet.push_back(tessControlShader->getShaderStageInfo());
		if (tessEvalShader) 
			shaderInfoSet.push_back(tessEvalShader->getShaderStageInfo());
		if (geometryShader) 
			shaderInfoSet.push_back(geometryShader->getShaderStageInfo());
		if (fragShader) 
			shaderInfoSet.push_back(fragShader->getShaderStageInfo());

		return shaderInfoSet;
	}
};

