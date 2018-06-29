#include <iostream>
#include "VkApp.h"

int main(int argc, char** argv)
{
	VkApp app;
	try {
		app.run();
	}
	catch (std::exception &e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	}
	std::cout << "press enter to complete" << std::endl;
	std::cin.get();
	return 0;
}