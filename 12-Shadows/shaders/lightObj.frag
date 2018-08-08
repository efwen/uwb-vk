#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 1) uniform AttachedLight
{
	vec4 position;
	vec4 direction;

	vec4 ambient;
	vec4 diffuse;
	vec4 specular;

	bool isEnabled;

	float constant;
	float linear;
	float quadratic;
	
	float cutOff;
    float outerCutOff;
} light;

layout(location = 0) out vec4 outFragColor;

void main() 
{
	float modif = 1.0;
	if(!light.isEnabled)	//dim the indicator if light is disabled
		modif = 0.1; 

	outFragColor = modif * light.diffuse;
}