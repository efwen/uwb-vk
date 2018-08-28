#pragma once

/*
PrintUtil.h
Utilities for printing debug statements
*/

#include <iostream>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

/** @brief print out a glm::vec2 object
	@param stream the std::ostream object to print out
	@param vec The glm::vec2 to print
	@return The ostream to print to
*/
std::ostream& operator<< (std::ostream& stream, const glm::vec2& vec) {
	stream << "(" << vec.x << ", " << vec.y << ")";
	return stream;
}

/** @brief print out a glm::vec3 object
	@param stream the std::ostream object to print out
	@param vec The glm::vec3 to print
	@return The ostream to print to
*/
std::ostream& operator<< (std::ostream& stream, const glm::vec3& vec) {
	stream << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
	return stream;
}

/** @brief print out a glm::vec4 object
	@param stream the std::ostream object to print out
	@param vec The glm::vec4 to print
	@return The ostream to print to
*/
std::ostream& operator<< (std::ostream& stream, const glm::vec4& vec) {
	stream << "(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
	return stream;
}

