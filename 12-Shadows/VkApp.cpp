#include "VkApp.h"


std::ostream& operator<< (std::ostream& stream, const glm::vec2& vec) {
	stream << "(" << vec.x << ", " << vec.y << ")";
	return stream;
}

std::ostream& operator<< (std::ostream& stream, const glm::vec3& vec) {
	stream << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
	return stream;
}

std::ostream& operator<< (std::ostream& stream, const glm::vec4& vec) {
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

		mCamera->updateViewMatrix();
		
		//light buffers
		mRenderSystem.updateUniformBuffer<LightUBO>(*mLightUBOBuffer, mLightUBO, 0);
		
		//update transform buffers
		updateMVPBuffer(*mWallMVPBuffer, *mWall, mWallXForm, *mCamera);
		
		//update light indicators
		for (uint32_t lightIndex = 0; lightIndex < MAX_LIGHTS; lightIndex++) {
			mLightIndicatorXForm[lightIndex].position = mLightUBO.lights[lightIndex].position;

			updateMVPBuffer(*mLightIndicatorMVPBuffer[lightIndex], *mLightIndicators[lightIndex], mLightIndicatorXForm[lightIndex], *mCamera);
			mRenderSystem.updateUniformBuffer<Light>(*mLightIndicatorLightBuffer[lightIndex], mLightUBO.lights[lightIndex], 0);
		}
		

		mRenderSystem.drawFrame();

		//update the frame timer
		mElapsedTime = (float)glfwGetTime();
		mFrameTime = mElapsedTime - mPrevTime;		
		mPrevTime = mElapsedTime;
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
	setupLights();
	
	createWall();

	for(uint32_t lightIndex = 0; lightIndex < MAX_LIGHTS; lightIndex++)
		createLightIndicator(lightIndex);
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
	mWindow = glfwCreateWindow(WIDTH, HEIGHT, "12-MultipleLights", nullptr, nullptr);

	if (mWindow == nullptr) throw std::runtime_error("Window creation failed!");
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
	float transDist = cCamTranslateSpeed * mFrameTime;
	float rotAmount = glm::radians(cCamRotateSpeed * mFrameTime);

	const float cursorSensitivity = 3.5f;
	glm::vec3 deltaCursor = mInputSystem.getMouseDelta();
	mCamera->rotation *= glm::angleAxis(-deltaCursor.x * cursorSensitivity * mFrameTime, mCamera->up);	//rotation is ccw around axis
	
	if (mInputSystem.isKeyDown(GLFW_KEY_W)) {
		mCamera->position += mCamera->forward * transDist;
	}
	if (mInputSystem.isKeyDown(GLFW_KEY_S)) {
		mCamera->position -= mCamera->forward * transDist;
	}
	if (mInputSystem.isKeyDown(GLFW_KEY_A)) {		
		mCamera->position -= mCamera->right * transDist;
	}
	if (mInputSystem.isKeyDown(GLFW_KEY_D)) {
		mCamera->position += mCamera->right * transDist;
	}
	if (mInputSystem.isKeyDown(GLFW_KEY_Q)) { //down
		mCamera->position -= mCamera->up * transDist;
	}
	if (mInputSystem.isKeyDown(GLFW_KEY_E)) { //up
		mCamera->position += mCamera->up * transDist;
	}


	if (mInputSystem.isKeyPressed(GLFW_KEY_C)) {
		std::cout << "cam vectors:" << std::endl;
		std::cout << "position: " << mCamera->position << std::endl;
		std::cout << "forward:  " << mCamera->forward << std::endl;
		std::cout << "right:    " << mCamera->right << std::endl;
		std::cout << "up:       " << mCamera->up << std::endl;
	}
	if (mInputSystem.isKeyPressed(GLFW_KEY_P))
	{
		std::cout << mCamera->position << std::endl;
	}
}


void VkApp::lightControls()
{
	float orbitRadius = 13.0f;
	float orbitSpeed = 0.5f;
	float wobbleSpeed = 4.0f;
	float orbitWobble = 5.0f;
	float offConst = 0.5f;
	if (mLightOrbit) {
			mLightUBO.lights[0].position = glm::vec4(orbitRadius * glm::sin(orbitSpeed * mElapsedTime),
				orbitWobble * glm::cos(wobbleSpeed * mElapsedTime),
				orbitRadius * glm::cos(orbitSpeed * mElapsedTime), 1.0f);
	}
	else
	{
		float transDist = cModelTranslateSpeed * mFrameTime;

		if (mInputSystem.isKeyDown(GLFW_KEY_UP)) {	//negative z is away from pov
			mLightUBO.lights[0].position += glm::vec4(0.0f, 0.0f, transDist, 1.0f);
		}
		if (mInputSystem.isKeyDown(GLFW_KEY_DOWN)) {
			mLightUBO.lights[0].position -= glm::vec4(0.0f, 0.0f, transDist, 1.0f);
		}
		if (mInputSystem.isKeyDown(GLFW_KEY_LEFT)) {	//negative z is away from pov
			mLightUBO.lights[0].position -= glm::vec4(transDist, 0.0f, 0.0f, 1.0f);
		}
		if (mInputSystem.isKeyDown(GLFW_KEY_RIGHT)) {
			mLightUBO.lights[0].position += glm::vec4(transDist, 0.0f, 0.0f, 1.0f);
		}
		if (mInputSystem.isKeyDown(GLFW_KEY_LEFT_BRACKET)) {	//negative z is away from pov
			mLightUBO.lights[0].position -= glm::vec4(0.0f, transDist, 0.0f, 1.0f);
		}
		if (mInputSystem.isKeyDown(GLFW_KEY_RIGHT_BRACKET)) {
			mLightUBO.lights[0].position += glm::vec4(0.0f, transDist, 0.0f, 1.0f);
		}
	}

	for (int lightIndex = 0; lightIndex < MAX_LIGHTS; lightIndex++)
	{
		if (mInputSystem.isKeyPressed(GLFW_KEY_0 + lightIndex)) {
			std::cout << lightIndex << " key clicked!" << std::endl;
			bool isEnabled = mLightUBO.lights[lightIndex].isEnabled;
			mLightUBO.lights[lightIndex].isEnabled = !isEnabled;
		}
	}

	for (uint32_t lightIndex = 1; lightIndex < MAX_LIGHTS; lightIndex++) {
		glm::quat rot = glm::angleAxis(glm::radians(360.0f / MAX_LIGHTS * lightIndex), glm::vec3(0.0f, 1.0f, 0.0f));
		mLightUBO.lights[lightIndex].position = rot * mLightUBO.lights[0].position;
	}
}

void VkApp::setupCamera()
{
	mCamera = std::make_unique<Camera>(Camera(WIDTH, HEIGHT));
	mCamera->position = glm::vec3(0.0f, 3.0f, 20.0f);
}

void VkApp::setupLights()
{
	mRenderSystem.createUniformBuffer<LightUBO>(mLightUBOBuffer, 1);

	//setup light #0 (point light)
	mLightUBO.lights[0].isEnabled = true;
	mLightUBO.lights[0].ambient = glm::vec4(0.1, 0.1, 0.1, 1.0);
	mLightUBO.lights[0].diffuse = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);		//cyan light
	mLightUBO.lights[0].specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);		//cyan light

	//setup light #1 (point light)
	mLightUBO.lights[1].isEnabled = true;
	mLightUBO.lights[1].ambient = glm::vec4(0.1, 0.1, 0.1, 1.0);
	mLightUBO.lights[1].diffuse = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);		//magenta light
	mLightUBO.lights[1].specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);		//cyan light
	
	//setup light #2 (point light)
	mLightUBO.lights[2].isEnabled = true;
	mLightUBO.lights[2].ambient = glm::vec4(0.1, 0.1, 0.1, 1.0);
	mLightUBO.lights[2].diffuse = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);		//yellow light
	mLightUBO.lights[2].specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);		//cyan light

	//setup light #3 (point light)
	mLightUBO.lights[3].isEnabled = true;
	mLightUBO.lights[3].ambient = glm::vec4(0.1, 0.1, 0.1, 1.0);
	mLightUBO.lights[3].diffuse = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);		//red light
	mLightUBO.lights[3].specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);		//cyan light
}

void VkApp::createLightIndicator(uint32_t lightIndex)
{
	//Resources
	std::shared_ptr<Mesh> lightMesh;
	mRenderSystem.createMesh(lightMesh, LIGHT_MODEL_PATH, false);
	
	ShaderSet lightIndicatorShaderSet;
	mRenderSystem.createShader(lightIndicatorShaderSet.vertShader, LIGHT_VERT_SHADER_PATH, VK_SHADER_STAGE_VERTEX_BIT);
	mRenderSystem.createShader(lightIndicatorShaderSet.fragShader, LIGHT_FRAG_SHADER_PATH, VK_SHADER_STAGE_FRAGMENT_BIT);

	mRenderSystem.createUniformBuffer<MVPMatrices>(mLightIndicatorMVPBuffer[lightIndex], 1);
	mRenderSystem.createUniformBuffer<Light>(mLightIndicatorLightBuffer[lightIndex], 1);


	mRenderSystem.createRenderable(mLightIndicators[lightIndex]);

	mLightIndicators[lightIndex]->applyShaderSet(lightIndicatorShaderSet);
	mLightIndicators[lightIndex]->addShaderBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0, 1);
	mLightIndicators[lightIndex]->addShaderBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1);


	mLightIndicators[lightIndex]->setMesh(lightMesh);
	mLightIndicators[lightIndex]->bindUniformBuffer(mLightIndicatorMVPBuffer[lightIndex], 0);
	mLightIndicators[lightIndex]->bindUniformBuffer(mLightIndicatorLightBuffer[lightIndex], 1);

	mLightIndicatorXForm[lightIndex].scale = glm::vec3(0.1f);

	std::cout << "Instantiating light #" << lightIndex << std::endl;
	mRenderSystem.instantiateRenderable(mLightIndicators[lightIndex]);
}

void VkApp::createWall()
{
	//start by creating the component resources
	std::shared_ptr<Mesh> wallMesh;
	mRenderSystem.createMesh(wallMesh, WALL_MODEL_PATH, true);

	std::shared_ptr<Texture> wallTexture;
	mRenderSystem.createTexture(wallTexture, WALL_TEXTURE_PATH);

	ShaderSet planeShaderSet;
	mRenderSystem.createShader(planeShaderSet.vertShader, WALL_VERT_SHADER_PATH, VK_SHADER_STAGE_VERTEX_BIT);
	mRenderSystem.createShader(planeShaderSet.fragShader, WALL_FRAG_SHADER_PATH, VK_SHADER_STAGE_FRAGMENT_BIT);
	
	mRenderSystem.createUniformBuffer<MVPMatrices>(mWallMVPBuffer, 1);

	//create a renderable and make the appropriate attachments
	mRenderSystem.createRenderable(mWall);

	//setup the shaders and note the bindings they will use
	//Current restriction: one resource per binding (no arrays right now) 
	mWall->applyShaderSet(planeShaderSet);	
	mWall->addShaderBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0, 1);				//MVP
	mWall->addShaderBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1);				//lights
	mWall->addShaderBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2, 1);		//color texture

	mWall->setMesh(wallMesh);


	//bind resources
	mWall->bindUniformBuffer(mWallMVPBuffer, 0);
	mWall->bindUniformBuffer(mLightUBOBuffer, 1);
	mWall->bindTexture(wallTexture, 2);

	//set some initial conditions
	mWallXForm.scale = glm::vec3(5.0f);
	//mWallXForm.rotation *= glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));

	//instantiate (flush bindings, create pipeline)
	std::cout << "Instantianting a wall" << std::endl;
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

	mRenderSystem.updateUniformBuffer<MVPMatrices>(mvpBuffer, mvp, 0);
}
