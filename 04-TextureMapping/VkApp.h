#pragma once

#include <iostream>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "RenderSystem.h"
#include "InputSystem.h"

const int WIDTH = 1280;
const int HEIGHT = 720;
const bool WINDOWED = true;
const std::string WINDOW_TITLE = "4-TextureMapping";

class VkApp
{
private:
	GLFWwindow* mWindow;
	RenderSystem renderer;
	InputSystem input;
	double mTime = 0.0;
	double mPrevTime = 0.0;
	double mFrameTime = 0.0;

	std::vector<VkClearValue> clearColors = {	{1.0, 0.0, 0.0, 1.0},		//R
												{0.0, 1.0, 0.0, 1.0},		//G
												{0.0, 0.0, 1.0, 1.0} };		//B
	int clearColorIndex = 0;
public:
	VkApp();
	void run();

private:
	void initialize();
	void shutdown();
	void createWindow();

	void handleInput();
};
