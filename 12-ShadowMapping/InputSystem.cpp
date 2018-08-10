#include "InputSystem.h"

void InputSystem::keyCallback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
	switch (action)
	{
	case GLFW_PRESS:
		mKeyDown[key] = true;
		break;
	case GLFW_RELEASE:
		mKeyDown[key] = false;
		break;
	}
}

void InputSystem::mouseButtonCallback(GLFWwindow * window, int button, int action, int mods)
{
	switch (action)
	{
	case GLFW_PRESS:
		mMouseDown[button] = true;
		break;
	case GLFW_RELEASE:
		mMouseDown[button] = false;
		break;
	}
}

void InputSystem::mouseCursorCallback(GLFWwindow * window, double xpos, double ypos)
{
	mCursorPos.x = xpos;
	mCursorPos.y = ypos;

	//used to prevent a violent delta when you first move the mouse
	if (!cursorSet) {
		mPrevCursorPos = mCursorPos;
		cursorSet = true;
	}
}

void InputSystem::mouseScrollCallback(GLFWwindow * window, double xoffset, double yoffset)
{
	mCursorPos.z += yoffset;
}

InputSystem* InputSystem::mEventHandleInstance;

InputSystem::InputSystem()
{
	for (bool key : mKeyDown) key = false;
	for (bool key : mKeyPrev) key = false;
	for (bool key : mKeyPressed) key = false;
	for (bool key : mKeyReleased) key = false;

	setEventHandling();
}

void InputSystem::initialize(GLFWwindow * window)
{
	mWindow = window;
	glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwSetKeyCallback(mWindow, 
		dispatchKeyCallback);

	glfwSetMouseButtonCallback(mWindow, 
		dispatchMouseButtonCallback);

	glfwSetCursorPosCallback(mWindow, 
		dispatchMousePosCallback);

	glfwSetScrollCallback(mWindow,
		dispatchMouseScrollCallback);
}

void InputSystem::update()
{
	for (int i = 0; i < GLFW_KEY_LAST; i++) {
		mKeyPressed[i] = !mKeyPrev[i] && mKeyDown[i];
		mKeyReleased[i] = mKeyPrev[i] && !mKeyDown[i];
		mKeyPrev[i] = mKeyDown[i];
	}

	for (int i = 0; i < GLFW_MOUSE_BUTTON_LAST; i++) {
		mMousePressed[i] = !mMousePrev[i] && mMouseDown[i];
		mMouseReleased[i] = mMousePrev[i] && !mMouseDown[i];
		mMousePrev[i] = mMouseDown[i];
	}

	mDeltaCursorPos = mCursorPos - mPrevCursorPos;
	mPrevCursorPos = mCursorPos;
}

bool InputSystem::isKeyDown(int key)
{
	return mKeyDown[key];
}

bool InputSystem::isKeyPressed(int key)
{
	return mKeyPressed[key];
}

bool InputSystem::isKeyReleased(int key)
{
	return mKeyReleased[key];
}

bool InputSystem::isMouseDown(int button)
{
	return mMouseDown[button];
}

bool InputSystem::isMousePressed(int button)
{
	return mMousePressed[button];
}

bool InputSystem::isMouseReleased(int button)
{
	return mMouseReleased[button];
}

glm::vec3 InputSystem::getMouseDelta()
{
	return mDeltaCursorPos;
}

glm::vec3 InputSystem::getMousePos()
{
	return mCursorPos;
}


