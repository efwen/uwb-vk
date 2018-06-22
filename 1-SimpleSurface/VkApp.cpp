#include "VkApp.h"


VkApp::VkApp()
{
	mWindow = nullptr;
}

void VkApp::run()
{
	init();
	
	//update loop
	while (!glfwWindowShouldClose(mWindow)) {
		glfwPollEvents();
	}

	shutdown();
}

void VkApp::init()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	mWindow = glfwCreateWindow(800, 600, "1-SimpleSurface", nullptr, nullptr);

	if (mWindow == nullptr) {
		throw std::runtime_error("Window creation failed!");
	}

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

}
