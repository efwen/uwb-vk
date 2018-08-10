#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

//Light type "enum"
const uint eLightType_None = 0;
const uint eLightType_Directional = 1;
const uint eLightType_Point = 2;
const uint eLightType_Spot = 3;

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
	uint  lightType;
} light;

layout(location = 0) out vec4 outFragColor;

void main() 
{
	float modif = (light.isEnabled) ? 1.0 : 0.1;

	outFragColor = modif * light.diffuse;
}