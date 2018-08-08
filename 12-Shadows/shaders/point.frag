#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

const int MAX_LIGHTS = 4;
const float shininess = 32.0;

struct Light
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
};

layout(binding = 1) uniform UBO
{	
	vec4 viewPos;
	Light lights[MAX_LIGHTS];
} ubo;

layout(binding = 2) uniform sampler2D textureMap;

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec4 outFragColor;

vec3 processDirLight(Light directionalLight, vec3 worldNormal, vec3 worldFragPos, vec3 viewDir);
vec3 processPointLight(Light pointLight, vec3 worldNormal, vec3 worldFragPos, vec3 viewDir);
vec3 processSpotLight(Light spotLight, vec3 worldNormal, vec3 worldFragPos, vec3 viewDir);

void main() 
{	
	vec3 viewDir = normalize(ubo.viewPos.xyz - inWorldPos);

	vec3 result = vec3(0.0);
	for(int i = 0; i < MAX_LIGHTS; i++)
	{
		if(!ubo.lights[i].isEnabled) continue;

		result += processSpotLight(ubo.lights[i], inNormal, inWorldPos, viewDir);
	}

	outFragColor = vec4(result, 1.0);
}


vec3 processDirLight(Light dirLight, vec3 worldNormal, vec3 worldFragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(-dirLight.direction.xyz);

	float diff = max(dot(lightDir, inNormal), 0.0);
	float spec = 0.0;
	if(diff > 0.0)
	{
		vec3 reflectDir = reflect(-lightDir, inNormal);
		spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	}

	vec3 ambient  =  dirLight.ambient.rgb * texture(textureMap, inUV).rgb;
	vec3 diffuse  =  dirLight.diffuse.rgb * diff * texture(textureMap, inUV).rgb;
	vec3 specular =  dirLight.specular.rgb * spec * vec3(1.0, 1.0, 1.0);

	return (ambient + diffuse + specular);
}

vec3 processPointLight(Light pointLight, vec3 worldNormal, vec3 worldFragPos, vec3 viewDir)
{
		vec3 lightDir = normalize(pointLight.position.xyz - inWorldPos);

		float diff = max(dot(lightDir, inNormal), 0.0);

		float spec = 0.0;
		if(diff > 0.0)
		{
			vec3 reflectDir = reflect(-lightDir, inNormal);
			spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
		}

		float dist = length(pointLight.position.xyz - inWorldPos);
		float attenuation = 1.0 / 
			(pointLight.constant + 
			 pointLight.linear * dist + 
			 pointLight.quadratic * (dist * dist));    
   
		vec3 ambient  =  pointLight.ambient.rgb * texture(textureMap, inUV).rgb;
		vec3 diffuse  =  pointLight.diffuse.rgb * diff * texture(textureMap, inUV).rgb;
		vec3 specular =  pointLight.specular.rgb * spec * vec3(1.0, 1.0, 1.0); //texture(textureMap, inUV).rgb;

		ambient *= attenuation;
		diffuse *= attenuation;
		specular *= attenuation;
		return (ambient + diffuse + specular);
}

vec3 processSpotLight(Light spotLight, vec3 worldNormal, vec3 worldFragPos, vec3 viewDir)
{
		vec3 lightDir = normalize(spotLight.position.xyz - inWorldPos);

		float diff = max(dot(lightDir, inNormal), 0.0);

		float spec = 0.0;
		if(diff > 0.0)
		{
			vec3 reflectDir = reflect(-lightDir, inNormal);
			spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
		}

		float dist = length(spotLight.position.xyz - inWorldPos);
		float attenuation = 1.0 / 
			(spotLight.constant + 
			 spotLight.linear * dist + 
			 spotLight.quadratic * (dist * dist));  

		float theta = dot(lightDir, normalize(-spotLight.direction.xyz)); 
    	float epsilon = spotLight.cutOff - spotLight.outerCutOff;
    	float intensity = clamp((theta - spotLight.outerCutOff) / epsilon, 0.0, 1.0);  
   
		vec3 ambient  =  spotLight.ambient.rgb * texture(textureMap, inUV).rgb;
		vec3 diffuse  =  spotLight.diffuse.rgb * diff * texture(textureMap, inUV).rgb;
		vec3 specular =  spotLight.specular.rgb * spec * vec3(1.0, 1.0, 1.0); //texture(textureMap, inUV).rgb;

		ambient *= attenuation * intensity;
		diffuse *= attenuation * intensity;
		specular *= attenuation * intensity;
		return (ambient + diffuse + specular);
}
