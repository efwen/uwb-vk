#include "VkApp.h"


std::ostream& operator<< (std::ostream& stream, const glm::vec3& vec)
{
	stream << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
	return stream;
}

std::ostream& operator<< (std::ostream& stream, const glm::vec4& vec)
{
	stream << "(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
	return stream;
}

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

		mCamera->updateViewMat();
		//update all of our buffers
		
		updateMVPBuffer(*mWallMVPBuffer, *mWall, mWallXForm, *mCamera);
		
		mLightIndicatorXForm.position = mLightPos;
		updateMVPBuffer(*mLightIndicatorMVPBuffer, *mLightIndicator, mLightIndicatorXForm, *mCamera);
		
		mRenderSystem.updateUniformBuffer<Light>(*mLightBuffer, mLight);
		mRenderSystem.updateUniformBuffer<glm::vec3>(*mLightPosBuffer, mLightPos);

		mRenderSystem.drawFrame();

		//update the frame timer
		mTime = glfwGetTime();
		mFrameTime = mTime - mPrevTime;		
		mPrevTime = mTime;
	}


	std::cout << "---------------------------------" << std::endl;
	shutdown();
}

void VkApp::initialize()
{
	glfwInit();
	createWindow();
	mRenderSystem.initialize(mWindow);
	mInputSystem.initialize(mWindow);
	
	setupCamera();
	setupLight();
	
	createWall();
	createLightIndicator();
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
	mWindow = glfwCreateWindow(WIDTH, HEIGHT, "11-Illumination", nullptr, nullptr);

	if (mWindow == nullptr) {
		throw std::runtime_error("Window creation failed!");
	}
}

void VkApp::handleInput()
{
	mInputSystem.update();

	//close the window
	if (mInputSystem.isKeyPressed(GLFW_KEY_ESCAPE))
		glfwSetWindowShouldClose(mWindow, GLFW_TRUE);

	//change the clear color
	if (mInputSystem.isKeyPressed(GLFW_KEY_B)) {
		mRenderSystem.setClearColor(clearColors[clearColorIndex]);
		clearColorIndex++;
		if (clearColorIndex >= clearColors.size()) clearColorIndex = 0;
	}

	//print out FPS
	if (mInputSystem.isKeyPressed(GLFW_KEY_F))
		std::cout << "frameTime: " << mFrameTime * 1000.0 << " ms ( " << (1.0 / mFrameTime) << " fps)" << std::endl;

	if (mInputSystem.isKeyPressed(GLFW_KEY_L))
		mLightOrbit = !mLightOrbit;
	cameraControls();
	lightControls();
}

void VkApp::cameraControls()
{
	float transDist = cCamTranslateSpeed * (float)mFrameTime;
	float rotAmount = glm::radians(cCamRotateSpeed * (float)mFrameTime);

	if (mInputSystem.isKeyDown(GLFW_KEY_W)) {
		mCamera->position += mCamera->forward * transDist;
	}
	if (mInputSystem.isKeyDown(GLFW_KEY_S)) {
		mCamera->position -= mCamera->forward * transDist;
	}
	if (mInputSystem.isKeyDown(GLFW_KEY_A)) {		
		mCamera->rotation *= glm::angleAxis(rotAmount, mCamera->up);
	}
	if (mInputSystem.isKeyDown(GLFW_KEY_D)) {
		mCamera->rotation *= glm::angleAxis(-rotAmount, mCamera->up);
	}
	if (mInputSystem.isKeyDown(GLFW_KEY_Q)) { //down
		mCamera->position -= mCamera->up * transDist;
	}
	if (mInputSystem.isKeyDown(GLFW_KEY_E)) { //up
		mCamera->position += mCamera->up * transDist;
	}

	if (mInputSystem.isKeyPressed(GLFW_KEY_P))
	{
		std::cout << mCamera->position << std::endl;
	}
}

void VkApp::lightControls()
{
	float orbitRadius = 5.0f;
	float orbitSpeed = 0.5f;
	float wobbleSpeed = 4.0f;
	float orbitWobble = 5.0f;
	if (mLightOrbit) {
		mLightPos = glm::vec4(orbitRadius * glm::sin(orbitSpeed * (float)mTime),
			orbitWobble * glm::cos(wobbleSpeed * (float)mTime),
			orbitRadius * glm::cos(orbitSpeed * (float)mTime), 0.0f);
	}
	else
	{

		float transDist = cModelTranslateSpeed * (float)mFrameTime;

		if (mInputSystem.isKeyDown(GLFW_KEY_UP)) {	//negative z is away from pov
			mLightPos += glm::vec3(0.0f, 0.0f, transDist);
		}
		if (mInputSystem.isKeyDown(GLFW_KEY_DOWN)) {
			mLightPos -= glm::vec3(0.0f, 0.0f, transDist);// mCamera.forward * transDist;
		}
		if (mInputSystem.isKeyDown(GLFW_KEY_LEFT)) {	//negative z is away from pov
			mLightPos -= glm::vec3(transDist, 0.0f, 0.0f);
		}
		if (mInputSystem.isKeyDown(GLFW_KEY_RIGHT)) {
			mLightPos += glm::vec3(transDist, 0.0f, 0.0f);// mCamera.forward * transDist;
		}
		if (mInputSystem.isKeyDown(GLFW_KEY_LEFT_BRACKET)) {	//negative z is away from pov
			mLightPos -= glm::vec3(0.0f, transDist, 0.0f);
		}
		if (mInputSystem.isKeyDown(GLFW_KEY_RIGHT_BRACKET)) {
			mLightPos += glm::vec3(0.0f, transDist, 0.0f);// mCamera.forward * transDist;
		}
	}

}

void VkApp::setupCamera()
{
	mCamera = std::make_unique<Camera>(Camera(WIDTH, HEIGHT));
	mCamera->position = glm::vec3(0.0f, -3.0f, 20.0f);
}

void VkApp::setupLight()
{
	mRenderSystem.createUniformBuffer<Light>(mLightBuffer);
	mRenderSystem.createUniformBuffer<glm::vec3>(mLightPosBuffer);

	mLight.ambient = glm::vec4(0.2f, 0.2f, 0.2f, 0.1f);
	mLight.diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	mLight.specular = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

	mLightPos = glm::vec4(0.0f, 5.0f, 0.0f, 0.0f);
}

void VkApp::createLightIndicator()
{
	//Resources
	std::shared_ptr<Mesh> lightMesh;
	mRenderSystem.createMesh(lightMesh, LIGHT_MODEL_PATH);

	mRenderSystem.createUniformBuffer<MVPMatrices>(mLightIndicatorMVPBuffer);

	ShaderSet lightIndicatorShaderSet;
	mRenderSystem.createShader(lightIndicatorShaderSet.vertShader, LIGHT_VERT_SHADER_PATH, VK_SHADER_STAGE_VERTEX_BIT);
	mRenderSystem.createShader(lightIndicatorShaderSet.fragShader, LIGHT_FRAG_SHADER_PATH, VK_SHADER_STAGE_FRAGMENT_BIT);


	//Renderable
	mRenderSystem.createRenderable(mLightIndicator);

	mLightIndicator->applyShaderSet(lightIndicatorShaderSet);
	mLightIndicator->addShaderBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0, 1);					//MVP

	mLightIndicator->setMesh(lightMesh);

	mLightIndicator->bindUniformBuffer(mLightIndicatorMVPBuffer, 0);

	mLightIndicatorXForm.scale = glm::vec3(0.1f);
	mRenderSystem.instantiateRenderable(mLightIndicator);

}

void VkApp::createWall()
{
	//start by creating the component resources
	std::shared_ptr<Mesh> wallMesh;
	mRenderSystem.createMesh(wallMesh, WALL_MODEL_PATH);

	std::shared_ptr<Texture> wallTexture;
	mRenderSystem.createTexture(wallTexture, WALL_TEXTURE_PATH);

	mRenderSystem.createUniformBuffer<MVPMatrices>(mWallMVPBuffer);
	
	ShaderSet planeShaderSet;
	mRenderSystem.createShader(planeShaderSet.vertShader, WALL_VERT_SHADER_PATH, VK_SHADER_STAGE_VERTEX_BIT);
	mRenderSystem.createShader(planeShaderSet.fragShader, WALL_FRAG_SHADER_PATH, VK_SHADER_STAGE_FRAGMENT_BIT);

	//create a renderable and make the appropriate attachments
	mRenderSystem.createRenderable(mWall);

	//setup the shaders and note the bindings they will use
	//Current restriction: one resource per binding (no arrays right now) 
	mWall->applyShaderSet(planeShaderSet);	
	mWall->addShaderBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0, 1);				//MVP
	mWall->addShaderBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1, 1);				//light position
	mWall->addShaderBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 2, 1);				//light
	mWall->addShaderBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3, 1);		//color texture

	mWall->setMesh(wallMesh);

	//bind resources
	mWall->bindUniformBuffer(mWallMVPBuffer, 0);
	mWall->bindUniformBuffer(mLightPosBuffer, 1);
	mWall->bindUniformBuffer(mLightBuffer, 2);
	mWall->bindTexture(wallTexture, 3);

	//set some initial conditions
	mWallXForm.scale = glm::vec3(2.0f);

	//instantiate (flush bindings, create pipeline)
	mRenderSystem.instantiateRenderable(mWall);
}

void VkApp::updateMVPBuffer(const UBO& mvpBuffer,
							const Renderable& renderable,    
							  const Transform& renderableXForm, 
							  const Camera& camera)
{
	MVPMatrices mvp = {};	
	mvp.model = renderableXForm.getModelMatrix();
	mvp.view = camera.viewMat;
	mvp.projection = camera.projMat;
	mvp.normalMat = glm::transpose(glm::inverse(mvp.view * mvp.model));

	mRenderSystem.updateUniformBuffer<MVPMatrices>(mvpBuffer, mvp);
}
