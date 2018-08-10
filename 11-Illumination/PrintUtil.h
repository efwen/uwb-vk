#pragma once

/*
PrintUtil.h
Utilities for printing debug statements
*/

#include <iostream>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

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

