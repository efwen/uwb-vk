#pragma once

#include <iostream>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "RenderSystem.h"
#include "InputSystem.h"

class VkApp
{
private:
	GLFWwindow* mWindow;
	RenderSystem renderer;
	InputSystem input;


	std::vector<VkClearValue> clearColors = {	{1.0, 0.0, 0.0, 1.0},		//R
												{0.0, 1.0, 0.0, 1.0},		//G
												{0.0, 0.0, 1.0, 1.0} };		//B
	int clearColorIndex = 0;
public:
	VkApp();
	void run();

private:
	void init();
	void shutdown();
	void createWindow();
};
