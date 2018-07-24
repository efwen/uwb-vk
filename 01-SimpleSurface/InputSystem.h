#pragma once
#include <GLFW/glfw3.h>



class InputSystem 
{
private:
	static InputSystem * mEventHandleInstance;
	GLFWwindow *mWindow = nullptr;

	bool mDown[GLFW_KEY_LAST];
	bool mPrevious[GLFW_KEY_LAST];
	bool mPressed[GLFW_KEY_LAST];
	bool mReleased[GLFW_KEY_LAST];
public:
	InputSystem();
	
	void initialize(GLFWwindow* window);
	void update();

	//status check
	bool isKeyDown(int key);
	bool isKeyPressed(int key);
	bool isKeyReleased(int key);
private:
	void setEventHandling() { mEventHandleInstance = this; }
	static void dispatchKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		if (mEventHandleInstance)
			mEventHandleInstance->keyCallback(window, key, scancode, action, mods);
	};

	void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};