#pragma once

#include <iostream>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/vec3.hpp>

#include "RenderSystem.h"
#include "InputSystem.h"

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

	float mCamDist = 0.0f;
	glm::vec3* mCamRotate = nullptr;
	const float mCamRotateSpeed = 100.0f;
	const float mCamTranslateSpeed = 10.0f;
public:
	VkApp();
	void run();

private:
	void initialize();
	void shutdown();
	void createWindow();
	void handleInput();
};
