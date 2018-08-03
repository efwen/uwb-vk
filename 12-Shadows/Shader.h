#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>
#include <memory>

#include "VulkanContext.h"

class Shader
{
public:
	Shader(std::shared_ptr<VulkanContext> context);
	~Shader();

	void load(const std::vector<char>& code, VkShaderStageFlagBits stage);
	void free();

	VkPipelineShaderStageCreateInfo getShaderStageInfo();
	VkShaderStageFlagBits getStage();
protected:
	std::shared_ptr<VulkanContext> mContext;

	//std::vector<char> mData;
	VkShaderStageFlagBits mStage = VK_SHADER_STAGE_ALL;
	VkShaderModule mShaderModule = VK_NULL_HANDLE;
	VkPipelineShaderStageCreateInfo mShaderStageInfo;

	void createShaderModule(const std::vector<char>& code);
};

struct ShaderSet {
	std::shared_ptr<Shader> vertShader = nullptr;
	std::shared_ptr<Shader> tessControlShader = nullptr;
	std::shared_ptr<Shader> tessEvalShader = nullptr;
	std::shared_ptr<Shader> geometryShader = nullptr;
	std::shared_ptr<Shader> fragShader = nullptr;

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

