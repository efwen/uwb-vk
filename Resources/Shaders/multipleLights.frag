#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

const uint MAX_LIGHTS = 8;

//Light type "enum"
const uint eLightType_None = 0;
const uint eLightType_Directional = 1;
const uint eLightType_Point = 2;
const uint eLightType_Spot = 3;

const float shininess = 16.0;

struct Light
{
	vec4  position;
	vec4  direction;

	vec4  ambient;
	vec4  diffuse;
	vec4  specular;

	bool  isEnabled;

	float constant;
	float linear;
	float quadratic;

	float cutOff;
    float outerCutOff;
	uint  lightType;
};

layout(binding = 1) uniform LightUBO
{	
	vec4 viewPos;
	Light lights[MAX_LIGHTS];
} ubo;

layout(binding = 2) uniform sampler2D textureMap;
layout(binding = 3) uniform sampler2D normalMap;
layout(Binding = 4) uniform sampler2D specularMap;

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in mat3 inWorldTBN;

layout(location = 0) out vec4 outFragColor;

//Forward declaration
vec3 processDirLight(Light directionalLight, vec3 worldNormal, vec3 worldFragPos, vec3 viewDir, vec3 diffuseColor, vec3 specularColor);
vec3 processPointLight(Light pointLight, vec3 worldNormal, vec3 worldFragPos, vec3 viewDir, vec3 diffuseColor, vec3 specularColor);
vec3 processSpotLight(Light spotLight, vec3 worldNormal, vec3 worldFragPos, vec3 viewDir, vec3 diffuseColor, vec3 specularColor);

void main() 
{	
	vec3 normal = texture(normalMap, inUV).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	normal = normalize(inWorldTBN * normal);

	vec3 viewDir = normalize(ubo.viewPos.xyz - inWorldPos);
	vec3 diffuseColor =  texture(textureMap, inUV).rgb;
	vec3 specularColor = texture(specularMap, inUV).rgb;//vec3(1.0, 1.0, 1.0);
	vec3 result = vec3(0.0);
	for(int i = 0; i < MAX_LIGHTS; i++)
	{
		if(!ubo.lights[i].isEnabled) continue;

		switch(ubo.lights[i].lightType)
		{
			case eLightType_Directional:
				result += processDirLight(ubo.lights[i], normal, inWorldPos, viewDir, diffuseColor, specularColor);
				break;
			case eLightType_Point:
				result += processPointLight(ubo.lights[i], normal, inWorldPos, viewDir, diffuseColor, specularColor);
				break;
			case eLightType_Spot:
				result += processSpotLight(ubo.lights[i], normal, inWorldPos, viewDir, diffuseColor, specularColor);
				break;
			default:		//invalid Light Type! notify user with solid color
				outFragColor = vec4(1.0, 0.0, 1.0, 1.0);
				return;
		}
	}
	outFragColor = vec4(result, 1.0);
}


vec3 processDirLight(Light dirLight, vec3 worldNormal, vec3 worldFragPos, vec3 viewDir, vec3 diffuseColor, vec3 specularColor)
{
	vec3 lightDir = normalize(-dirLight.direction.xyz);

	float diff = max(dot(lightDir, inWorldTBN[2]), 0.0);		//use the raw normal, so that we don't get reflections on unlit surfaces
	float spec = 0.0;
	if(diff > 0.0)
	{
		vec3 reflectDir = reflect(-lightDir, worldNormal);
		spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	}

	vec3 ambient  =  dirLight.ambient.rgb * diffuseColor;
	vec3 diffuse  =  dirLight.diffuse.rgb * diff * diffuseColor;
	vec3 specular =  dirLight.specular.rgb * spec * specularColor;

	return (ambient + diffuse + specular);
}

vec3 processPointLight(Light pointLight, vec3 worldNormal, vec3 worldFragPos, vec3 viewDir, vec3 diffuseColor, vec3 specularColor)
{
		vec3 lightDir = normalize(pointLight.position.xyz - worldFragPos);

		float diff = max(dot(lightDir, inWorldTBN[2]), 0.0);

		float spec = 0.0;
		if(diff > 0.0)
		{
			vec3 reflectDir = reflect(-lightDir, worldNormal);
			spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
		}

		float dist = length(pointLight.position.xyz - worldFragPos);
		float attenuation = 1.0 / 
			(pointLight.constant + 
			 pointLight.linear * dist + 
			 pointLight.quadratic * (dist * dist));    
   
		vec3 ambient  =  pointLight.ambient.rgb * diffuseColor;
		vec3 diffuse  =  pointLight.diffuse.rgb * diff * diffuseColor;
		vec3 specular =  pointLight.specular.rgb * spec * specularColor;

		ambient *= attenuation;
		diffuse *= attenuation;
		specular *= attenuation;
		return (ambient + diffuse + specular);
}

vec3 processSpotLight(Light spotLight, vec3 worldNormal, vec3 worldFragPos, vec3 viewDir,  vec3 diffuseColor, vec3 specularColor)
{
		vec3 lightDir = normalize(spotLight.position.xyz - worldFragPos);

		float diff = max(dot(lightDir, inWorldTBN[2]), 0.0); 

		float spec = 0.0;
		if(diff > 0.0)
		{
			vec3 reflectDir = reflect(-lightDir, worldNormal);
			spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
		}

		float dist = length(spotLight.position.xyz - worldFragPos);
		float attenuation = 1.0 / 
			(spotLight.constant + 
			 spotLight.linear * dist + 
			 spotLight.quadratic * (dist * dist));  

		float theta = dot(lightDir, normalize(-spotLight.direction.xyz)); 
    	float epsilon = spotLight.cutOff - spotLight.outerCutOff;
    	float intensity = clamp((theta - spotLight.outerCutOff) / epsilon, 0.0, 1.0);  
   
		vec3 ambient  =  spotLight.ambient.rgb * diffuseColor;
		vec3 diffuse  =  spotLight.diffuse.rgb * diff * diffuseColor;
		vec3 specular =  spotLight.specular.rgb * spec * specularColor;

		//ambient *= attenuation;
		diffuse *= attenuation * intensity;
		specular *= attenuation * intensity;
		return (ambient + diffuse + specular);
}
