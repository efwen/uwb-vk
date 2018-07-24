#include "Shader.h"
#include <assert.h>

Shader::Shader(std::shared_ptr<VulkanContext> context) :
	mContext(context),
	mStage(VK_SHADER_STAGE_ALL),
	mShaderModule(VK_NULL_HANDLE)
{}

Shader::~Shader()
{}

void Shader::load(const std::vector<char>& code, VkShaderStageFlagBits stage)
{
	assert(stage != VK_SHADER_STAGE_ALL &&
		stage != VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM);
	mStage = stage;

	createShaderModule(code);
}

void Shader::free()
{
	vkDestroyShaderModule(mContext->device, mShaderModule, nullptr);
	mStage = VK_SHADER_STAGE_ALL;
}

VkPipelineShaderStageCreateInfo Shader::getShaderStageInfo()
{
	VkPipelineShaderStageCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	createInfo.stage = mStage;
	createInfo.module = mShaderModule;
	createInfo.pName = "main";

	return createInfo;
}

VkShaderStageFlagBits Shader::getStage()
{
	return mStage;
}

void Shader::createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	if (vkCreateShaderModule(mContext->device, &createInfo, nullptr, &mShaderModule) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create shader module!");
	}
}
