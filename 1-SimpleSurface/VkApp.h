#pragma once

#include <iostream>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "RenderSystem.h"

class VkApp
{
private:
	GLFWwindow* mWindow;
	RenderSystem renderer;
public:
	VkApp();
	void run();

private:
	void init();
	void shutdown();
	void createWindow();

};
