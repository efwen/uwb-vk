#include "InputSystem.h"

void InputSystem::keyCallback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
	switch (action)
	{
	case GLFW_PRESS:
		mDown[key] = true;
		break;
	case GLFW_RELEASE:
		mDown[key] = false;
		break;
	}
}

InputSystem* InputSystem::mEventHandleInstance;

InputSystem::InputSystem()
{
	for (bool key : mDown) key = false;
	for (bool key : mPrevious) key = false;
	for (bool key : mPressed) key = false;
	for (bool key : mReleased) key = false;

	setEventHandling();
}

void InputSystem::initialize(GLFWwindow * window)
{
	mWindow = window;
	glfwSetKeyCallback(mWindow, 
		dispatchKeyCallback);
}

void InputSystem::update()
{
	for (int i = 0; i < GLFW_KEY_LAST; i++) {
		mPressed[i] = !mPrevious[i] && mDown[i];
		mReleased[i] = mPrevious[i] && !mDown[i];
		mPrevious[i] = mDown[i];
	}
}

bool InputSystem::isKeyDown(int key)
{
	return mDown[key];
}

bool InputSystem::isKeyPressed(int key)
{
	return mPressed[key];
}

bool InputSystem::isKeyReleased(int key)
{
	return mReleased[key];
}


