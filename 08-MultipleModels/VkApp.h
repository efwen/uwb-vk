#pragma once

#include <string>
#include <iostream>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/vec3.hpp>

#include "RenderSystem.h"
#include "InputSystem.h"
#include "Model.h"

const std::string CHALET_TEXTURE_PATH = "Resources/Textures/chalet.jpg";
const std::string GROUND_TEXTURE_PATH = "Resources/Textures/UrbanTexturePack/Ground_Dirt/Ground_Dirt_1k_d.tga";
const std::string CHALET_MODEL_PATH = "Resources/Meshes/chalet.mesh";
const std::string GROUND_MODEL_PATH = "Resources/Meshes/plane.mesh";

class VkApp
{
private:
	GLFWwindow* mWindow;
	RenderSystem mRenderSystem;
	InputSystem mInputSystem;
	double mTime = 0.0;
	double mPrevTime = 0.0;
	double mFrameTime = 0.0;

	std::vector<VkClearValue> clearColors = {   {0.937f, 0.749f, 0.376f, 1.0f},
												{0.788f, 0.2f,   0.125f, 1.0f},
												{0.176f, 0.11f,  0.114f, 1.0f},
												{0.0f,   0.0f,   0.0f,   1.0f } };		
	int clearColorIndex = 0;

	//"Camera"/View
	float mCamDist = 0.0f;
	glm::vec3* mCamRotate = nullptr;
	const float mCamTranslateSpeed = 10.0f;
	const float mCamRotateSpeed = 100.0f;

	//Models
	std::shared_ptr<Model> mTestModel = nullptr;
	const float mModelTranslateSpeed = 5.0f;
	const float mModelRotateSpeed = 100.0f;

	std::shared_ptr<Model> mTestPlane = nullptr;
public:
	VkApp();
	void run();

private:
	void initialize();
	void shutdown();
	void createWindow();
	void handleInput();
};
