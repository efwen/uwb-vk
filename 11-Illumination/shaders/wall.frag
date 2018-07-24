#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 2) uniform sampler2D texSampler;
layout(binding = 1) uniform LIGHT
{
	vec4 ambient;
}  light;

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec4 outFragColor;

void main() 
{
	vec4 scatteredLight = light.ambient;
	vec4 texColor = texture(texSampler, inUV);
	outFragColor = min(texColor * scatteredLight, vec4(1.0));
}