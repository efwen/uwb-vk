#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 3) uniform sampler2D texSampler[];

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec4 outFragColor;

void main() 
{
	outFragColor = vec4(texture(texSampler[0], inUV).rgb, 1.0);
}