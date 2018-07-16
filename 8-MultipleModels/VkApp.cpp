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

	mCamDist = mRenderSystem.getCamDist();
	mCamRotate = mRenderSystem.getCamRotate();

	mTestPlane = mRenderSystem.createModel(GROUND_MODEL_PATH, GROUND_TEXTURE_PATH);
	mTestModel = mRenderSystem.createModel(CHALET_MODEL_PATH, CHALET_TEXTURE_PATH);

	mTestPlane->mScale = glm::vec3(2.0f, 2.0f, 1.0f);
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
	mWindow = glfwCreateWindow(800, 600, "8-MultipleModels", nullptr, nullptr);

	if (mWindow == nullptr) {
		throw std::runtime_error("Window creation failed!");
	}
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

	//print out FPS
	if (mInputSystem.isKeyPressed(GLFW_KEY_F))
		std::cout << "frameTime: " << mFrameTime * 1000.0 << " ms ( " << (1.0 / mFrameTime) << " fps)" << std::endl;


	//Model Controls
	if (mInputSystem.isKeyDown(GLFW_KEY_J)) {
		mTestModel->mPosition -= glm::vec3(mModelTranslateSpeed * mFrameTime, 0.0f, 0.0f);
	}
	if (mInputSystem.isKeyDown(GLFW_KEY_L)) {
		mTestModel->mPosition += glm::vec3(mModelTranslateSpeed * mFrameTime, 0.0f, 0.0f);
	}
	if (mInputSystem.isKeyDown(GLFW_KEY_I)) {
		mTestModel->mPosition += glm::vec3(0.0f, mModelTranslateSpeed * mFrameTime, 0.0f);
	}
	if (mInputSystem.isKeyDown(GLFW_KEY_K)) {
		mTestModel->mPosition -= glm::vec3(0.0f, mModelTranslateSpeed * mFrameTime, 0.0f);
	}
	if (mInputSystem.isKeyDown(GLFW_KEY_U)) {
		mTestModel->zRotation += mModelRotateSpeed * mFrameTime;
	}
	if (mInputSystem.isKeyDown(GLFW_KEY_O)) {
		mTestModel->zRotation -= mModelRotateSpeed * mFrameTime;
	}


	//Camera Controls
	if (mInputSystem.isKeyDown(GLFW_KEY_UP)){
		mCamRotate->x += mCamRotateSpeed * mFrameTime;
	}
	if (mInputSystem.isKeyDown(GLFW_KEY_DOWN)){
		mCamRotate->x -= mCamRotateSpeed * mFrameTime;
	}
	if (mInputSystem.isKeyDown(GLFW_KEY_LEFT)) {
		mCamRotate->z += mCamRotateSpeed * mFrameTime;
	}
	if (mInputSystem.isKeyDown(GLFW_KEY_RIGHT)) {
		mCamRotate->z -= mCamRotateSpeed * mFrameTime;
	}
	//forward / back
	if (mInputSystem.isKeyDown(GLFW_KEY_W)) {
		mCamDist -= mCamTranslateSpeed * mFrameTime;
		mRenderSystem.setCamDist(mCamDist);
	}
	else if (mInputSystem.isKeyDown(GLFW_KEY_S)) {
		mCamDist += mCamTranslateSpeed * mFrameTime;
		mRenderSystem.setCamDist(mCamDist);
	}
}
