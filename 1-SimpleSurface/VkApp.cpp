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
		renderer.drawFrame();
	}

	std::cout << "---------------------------------------" << std::endl;

	shutdown();
}

void VkApp::init()
{
	glfwInit();
	createWindow();
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
