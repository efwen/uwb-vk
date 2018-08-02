#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable


layout(binding = 2) uniform PointLight
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
}  light;

layout(binding = 3) uniform sampler2D textureMap;
layout(binding = 4) uniform sampler2D normalMap;

layout(location = 0) in vec3 inFragPos;	//the fragment position in view space
layout(location = 1) in vec2 inUV;		//uv coord for texture
layout(location = 2) in vec3 inLightPos;
layout(location = 3) in mat3 inTBN;

layout(location = 0) out vec4 outFragColor;

void main() 
{	
	float ambientStrength = 0.1;
	float specularStrength = 0.5;

	vec3 lightDirection = normalize(inLightPos - inFragPos);
	
	//apply the normal map to our given normal	
	vec3 normal = texture(normalMap, inUV).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	normal = normalize(inTBN * normal);

	//diffuse
	//depends on the angle the light source makes with the normal
	float diff = max(dot(lightDirection, normal), 0.0);
	
	//specularity should only work if the light is on our side
	//depends on the angle between the view direction of a
	//fragment and the angle the light makes when bouncing off of that
	//fragment
	float spec = 0.0;
	if(diff > 0.0)
	{		
		vec3 viewDirection = normalize(-inFragPos);					//since inFragPos is in view space, the view position is (0, 0, 0)
		vec3 reflectDirection = reflect(-lightDirection, normal);

		float specAngle = max(dot(reflectDirection, viewDirection), 0.0);
		spec = pow(specAngle, 32.0);
	}

	vec3 ambient = ambientStrength * light.ambient.rgb;
	vec3 diffuse = diff * light.diffuse.rgb;
	vec3 specular = specularStrength * spec * light.specular.rgb;

	vec3 result = (ambient + diffuse + specular) * texture(textureMap
, inUV).rgb;
	outFragColor = vec4(result, 1.0);
}