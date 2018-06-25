#include "VkApp.h"


VkApp::VkApp()
{
	mWindow = nullptr;
}

void VkApp::run()
{
	init();
	
	std::cout << "Starting loop..." << std::endl;
	//update loop
	while (!glfwWindowShouldClose(mWindow)) {
		glfwPollEvents();
		input.update();

		//change the clear color
		if (input.isKeyPressed(GLFW_KEY_B)) {
			renderer.setClearColor(clearColors[clearColorIndex]);
			clearColorIndex++;
			if (clearColorIndex >= clearColors.size()) clearColorIndex = 0;
		}

		//close the window
		if (input.isKeyPressed(GLFW_KEY_ESCAPE)) 
			glfwSetWindowShouldClose(mWindow, GLFW_TRUE);

		renderer.drawFrame();
	}

	std::cout << "---------------------------------------" << std::endl;
	shutdown();
}

void VkApp::init()
{
	glfwInit();
	createWindow();
	input.initialize(mWindow);
	renderer.init(mWindow);
}

void VkApp::shutdown()
{
	renderer.shutdown();
	glfwDestroyWindow(mWindow);
	glfwTerminate();
}

void VkApp::createWindow()
{ 
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	mWindow = glfwCreateWindow(800, 600, "1-SimpleSurface", nullptr, nullptr);

	if (mWindow == nullptr) {
		throw std::runtime_error("Window creation failed!");
	}
}
