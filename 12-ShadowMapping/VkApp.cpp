#include "VkApp.h"
#include "PrintUtil.h"

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

		//mRenderSystem.setLightMVPBuffer()
		mCamera->updateViewMatrix();
		
		//light buffers
		mLightUBO.viewPos = glm::vec4(mCamera->position, 1.0);
		mRenderSystem.updateUniformBuffer<LightUBO>(*mLightUBOBuffer, mLightUBO, 0);
		
		//update transform buffers
		updateMVPBuffer(*mCubeMVPBuffer, *mCube, mCubeXForm, *mCamera);
		updateMVPBuffer(*mGroundMVPBuffer, *mGround, mGroundXForm, *mCamera);
		
		//update light indicators
		for (uint32_t lightIndex = 0; lightIndex < mTotalLights; lightIndex++) {
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
	
	createCube();
	mCubeXForm.scale = glm::vec3(4.0f);
	mCubeXForm.position += glm::vec3(0.0f, 2.0f, 0.0f);

	createGround();
	mGroundXForm.scale = glm::vec3(40.0f);
	mGroundXForm.rotation *= glm::angleAxis(glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	
	for(uint32_t lightIndex = 0; lightIndex < mTotalLights; lightIndex++)
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
}


void VkApp::lightControls()
{
	float orbitRadius = 30.0f;
	float orbitSpeed = 0.5f;
	float wobbleSpeed = 4.0f;
	float orbitWobble = 5.0f;
	float offConst = 0.5f;
	float transDist = cLightTranslateSpeed * mFrameTime;

	//in orbit mode, all of the lights orbit around the ori
	if (mLightOrbit) {
		mLightUBO.lights[0].position = glm::vec4(orbitRadius * glm::sin(orbitSpeed * mElapsedTime),
												6.0f,
												orbitRadius * glm::cos(orbitSpeed * mElapsedTime), 1.0f);

		mLightUBO.lights[0].direction = -glm::normalize(mLightUBO.lights[0].position);

		for (uint32_t lightIndex = 1; lightIndex < mTotalLights; lightIndex++) {
			glm::quat rot = glm::angleAxis(glm::radians(360.0f / mTotalLights * lightIndex), glm::vec3(0.0f, 1.0f, 0.0f));
			mLightUBO.lights[lightIndex].position = rot * mLightUBO.lights[0].position;
		}
	}
	else {	//if we're not in orbit mode, we can manipulate individual lights
		//move the selected light
		
		if (mInputSystem.isKeyDown(GLFW_KEY_UP)) {	//negative z is away from pov
			mLightUBO.lights[mSelectedLight].position += glm::vec4(0.0f, 0.0f, transDist, 1.0f);
		}
		if (mInputSystem.isKeyDown(GLFW_KEY_DOWN)) {
			mLightUBO.lights[mSelectedLight].position -= glm::vec4(0.0f, 0.0f, transDist, 1.0f);
		}
		if (mInputSystem.isKeyDown(GLFW_KEY_LEFT)) {	//negative z is away from pov
			mLightUBO.lights[mSelectedLight].position -= glm::vec4(transDist, 0.0f, 0.0f, 1.0f);
		}
		if (mInputSystem.isKeyDown(GLFW_KEY_RIGHT)) {
			mLightUBO.lights[mSelectedLight].position += glm::vec4(transDist, 0.0f, 0.0f, 1.0f);
		}
		if (mInputSystem.isKeyDown(GLFW_KEY_LEFT_BRACKET)) {	//negative z is away from pov
			mLightUBO.lights[mSelectedLight].position -= glm::vec4(0.0f, transDist, 0.0f, 1.0f);
		}
		if (mInputSystem.isKeyDown(GLFW_KEY_RIGHT_BRACKET)) {
			mLightUBO.lights[mSelectedLight].position += glm::vec4(0.0f, transDist, 0.0f, 1.0f);
		}

		//turn the selected light on/off
		if (mInputSystem.isKeyPressed(GLFW_KEY_O)) {
			mLightUBO.lights[mSelectedLight].isEnabled = !mLightUBO.lights[mSelectedLight].isEnabled;
			std::cout << "Light " << mSelectedLight << ": " << ((mLightUBO.lights[mSelectedLight].isEnabled) ? "on" : "off") << std::endl;
		}

		if (mInputSystem.isKeyPressed(GLFW_KEY_U)) {
			std::cout << "light position: " << mLightUBO.lights[mSelectedLight].position << std::endl;
		}
	}


	if (mInputSystem.isKeyDown(GLFW_KEY_G)) {
		mCubeXForm.position += glm::vec3(1.0, 0.0, 0.0) * transDist;
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
	mTotalLights = 1;

	//setup spotlight
	mLightUBO.lights[0].isEnabled	= true;
	mLightUBO.lights[0].lightType   = LightType::Spot;
	mLightUBO.lights[0].ambient     = glm::vec4(0.1, 0.1, 0.1, 1.0);
	mLightUBO.lights[0].diffuse     = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);		//red light
	mLightUBO.lights[0].specular    = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);		
	mLightUBO.lights[0].cutOff		= glm::cos(glm::radians(12.5f));
	mLightUBO.lights[0].outerCutOff = glm::cos(glm::radians(15.0f));
	mLightUBO.lights[0].constant = 0.25f;// 1.0f
	mLightUBO.lights[0].linear = 0.0f;// 0.09f;
	mLightUBO.lights[0].quadratic = 0.0f;// 0.032f;
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

void VkApp::createCube()
{
	//start by creating the component resources
	std::shared_ptr<Mesh> cubeMesh;
	mRenderSystem.createMesh(cubeMesh, BOX_MODEL_PATH, true);

	std::shared_ptr<Texture> cubeDiffuseMap;
	mRenderSystem.createTexture(cubeDiffuseMap, BOX_DIFFUSE_PATH);

	std::shared_ptr<Texture> cubeNormalMap;
	mRenderSystem.createTexture(cubeNormalMap, BOX_NORMAL_PATH);

	std::shared_ptr<Texture> cubeSpecularMap;
	mRenderSystem.createTexture(cubeSpecularMap, BOX_SPECULAR_PATH);

	ShaderSet cubeShaderSet;
	mRenderSystem.createShader(cubeShaderSet.vertShader, BOX_VERT_SHADER_PATH, VK_SHADER_STAGE_VERTEX_BIT);
	mRenderSystem.createShader(cubeShaderSet.fragShader, BOX_FRAG_SHADER_PATH, VK_SHADER_STAGE_FRAGMENT_BIT);
	
	mRenderSystem.createUniformBuffer<MVPMatrices>(mCubeMVPBuffer, 1);

	//create a renderable and make the appropriate attachments
	mRenderSystem.createRenderable(mCube);

	//setup the shaders and note the bindings they will use
	//Current restriction: one resource per binding (no arrays right now) 
	mCube->applyShaderSet(cubeShaderSet);	
	mCube->addShaderBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0, 1);				//MVP
	mCube->addShaderBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1);				//lights
	mCube->addShaderBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2, 1);		//diffuse map
	mCube->addShaderBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3, 1);		//normal map
	mCube->addShaderBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 4, 1);		//specular map

	mCube->setMesh(cubeMesh);

	//bind resources
	mCube->bindUniformBuffer(mCubeMVPBuffer, 0);
	mCube->bindUniformBuffer(mLightUBOBuffer, 1);
	mCube->bindTexture(cubeDiffuseMap, 2);
	mCube->bindTexture(cubeNormalMap, 3);
	mCube->bindTexture(cubeSpecularMap, 4);

	//instantiate (flush bindings, create pipeline)
	std::cout << "Instantianting a wall" << std::endl;
	mRenderSystem.instantiateRenderable(mCube);
}

void VkApp::createGround()
{
	//start by creating the component resources
	std::shared_ptr<Mesh> groundMesh;
	mRenderSystem.createMesh(groundMesh, GROUND_MESH_PATH, true);

	std::shared_ptr<Texture> groundDiffuseMap;
	mRenderSystem.createTexture(groundDiffuseMap, GROUND_DIFFUSE_PATH);

	std::shared_ptr<Texture> groundNormalMap;
	mRenderSystem.createTexture(groundNormalMap, GROUND_NORMAL_PATH);

	std::shared_ptr<Texture> groundSpecularMap;
	mRenderSystem.createTexture(groundSpecularMap, GROUND_SPECULAR_PATH);

	ShaderSet groundShaderSet;
	mRenderSystem.createShader(groundShaderSet.vertShader, GROUND_VERT_SHADER_PATH, VK_SHADER_STAGE_VERTEX_BIT);
	mRenderSystem.createShader(groundShaderSet.fragShader, GROUND_FRAG_SHADER_PATH, VK_SHADER_STAGE_FRAGMENT_BIT);

	mRenderSystem.createUniformBuffer<MVPMatrices>(mGroundMVPBuffer, 1);

	//create a renderable and make the appropriate attachments
	mRenderSystem.createRenderable(mGround);

	//setup the shaders and note the bindings they will use
	//Current restriction: one resource per binding (no arrays right now) 
	mGround->applyShaderSet(groundShaderSet);
	mGround->addShaderBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0, 1);				//MVP
	mGround->addShaderBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1);				//lights
	mGround->addShaderBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2, 1);		//diffuse map
	mGround->addShaderBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3, 1);		//normal map
	mGround->addShaderBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 4, 1);		//specular map

	mGround->setMesh(groundMesh);

	//bind resources
	mGround->bindUniformBuffer(mGroundMVPBuffer, 0);
	mGround->bindUniformBuffer(mLightUBOBuffer, 1);
	mGround->bindTexture(groundDiffuseMap, 2);
	mGround->bindTexture(groundNormalMap, 3);
	mGround->bindTexture(groundSpecularMap, 4);

	//instantiate (flush bindings, create pipeline)
	std::cout << "Instantianting a wall" << std::endl;
	mRenderSystem.instantiateRenderable(mGround);
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

void VkApp::updateShadowMVP(const Light & light)
{
	glm::mat4 lightMVP = glm::mat4(1.0);
	lightMVP = glm::translate(lightMVP, -glm::vec3(light.position));
	lightMVP = glm::lookAt(glm::vec3(light.position), glm::vec3(light.position + light.direction), glm::vec3(0.0, 1.0, 0.0));
	mRenderSystem.setLightMVPBuffer(lightMVP);
}
