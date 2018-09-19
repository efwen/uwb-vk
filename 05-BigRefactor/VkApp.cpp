#include "VkApp.h"


VkApp::VkApp() : mWindow(nullptr) {}

void VkApp::run()
{
	initialize();
	
	glfwSetTime(0.0);
	std::cout << "Starting loop..." << std::endl;
	//update loop
	while (!glfwWindowShouldClose(mWindow)) {
		glfwPollEvents();
		handleInput();

		mRenderSystem.drawFrame();

		//update the frame timer
		mTime = glfwGetTime();
		mFrameTime = mTime - mPrevTime;		
		mPrevTime = mTime;
	}

	std::cout << "---------------------------------------" << std::endl;
	shutdown();
}

void VkApp::initialize()
{
	glfwInit();
	createWindow();
	mInputSystem.initialize(mWindow);
	mRenderSystem.initialize(mWindow);
}

void VkApp::shutdown()
{
	mRenderSystem.cleanup();
	glfwDestroyWindow(mWindow);
	glfwTerminate();
}

void VkApp::createWindow()
{
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	GLFWmonitor* monitor = (WINDOWED ? nullptr : glfwGetPrimaryMonitor());
	mWindow = glfwCreateWindow(WIDTH, HEIGHT, WINDOW_TITLE.c_str(), monitor, nullptr);

	if (mWindow == nullptr) throw std::runtime_error("Window creation failed!");
}

void VkApp::handleInput()
{
	mInputSystem.update();

	//change the clear color
	if (mInputSystem.isKeyPressed(GLFW_KEY_B)) {
		mRenderSystem.setClearColor(clearColors[clearColorIndex]);
		clearColorIndex++;
		if (clearColorIndex >= clearColors.size()) clearColorIndex = 0;
	}

	//close the window
	if (mInputSystem.isKeyPressed(GLFW_KEY_ESCAPE))
		glfwSetWindowShouldClose(mWindow, GLFW_TRUE);

	if (mInputSystem.isKeyPressed(GLFW_KEY_F))
		std::cout << "frameTime: " << mFrameTime * 1000.0 << " ms ( " << (1.0 / mFrameTime) << " fps)" << std::endl;
}
