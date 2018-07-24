#pragma once

//Instance Extensions
const std::vector<const char*> instanceExtensions = {
	VK_KHR_SURFACE_EXTENSION_NAME
#ifdef _DEBUG
	VK_EXT_DEBUG_REPORT_EXTENSION_NAME
#endif
};

//Device Extensions
const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};