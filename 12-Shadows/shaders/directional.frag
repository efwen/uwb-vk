#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

const float shininess = 32.0;
const float strength = 4.0;
//uniform bindings
layout(binding = 2) uniform LightProperties
{
	bool isEnabled;
	bool isLocal;
	bool isSpot;	
	float spotCosineCutoff;
	float spotExponent;
	float constant;
	float linear;
	float quadratic;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
} light;

layout(binding = 3) uniform sampler2D textureMap;
layout(binding = 4) uniform sampler2D normalMap;
//layout(binding = 5) uniform MaterialProperties material;

//Material

//inputs
layout(location = 0) in vec3 inFragPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in mat3 inTBN;
layout(location = 6) in vec4 inLightPos;
layout(location = 7) in vec4 inLightDir;

layout(location = 0) out vec4 outFragColor;

void main() 
{	
	float ambientStrength = 0.1;
	float specularStrength = 0.5;

	vec3 lightDirection = normalize(vec3(-inLightDir));

	//apply the normal map to our given normal	
	vec3 normal = texture(normalMap, inUV).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	normal = normalize(inTBN * normal);

	//diffuse
	//depends on the angle the light source makes with the normal
	float diffuse = max(dot(lightDirection, normal), 0.0);
	
	//specularity should only work if the light is on our side
	//depends on the angle between the view direction of a
	//fragment and the angle the light makes when bouncing off of that
	//fragment
	float specular = 0.0;
	if(diffuse > 0.0)
	{		
		vec3 viewDirection = normalize(-inFragPos);					//since inFragPos is in view space, the view position is (0, 0, 0)
		vec3 reflectDirection = reflect(-lightDirection, normal);

		float specAngle = max(dot(reflectDirection, viewDirection), 0.0);
		specular = pow(specAngle, shininess);
	}

	vec3 scatteredLight = light.ambient.rgb + light.diffuse.rgb * diffuse;
	vec3 reflectedLight = light.specular.rgb * specular * specularStrength;

	vec3 result =  min(texture(textureMap, inUV).rgb * scatteredLight + reflectedLight, vec3(1.0));
	outFragColor = vec4(result, 1.0);
}