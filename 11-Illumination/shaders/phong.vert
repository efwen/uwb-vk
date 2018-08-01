#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable


layout(binding = 0) uniform MVP 
{
    mat4 projection;
    mat4 model;
    mat4 view;
    mat4 normalMat;
} mvp;

layout(binding = 1) uniform LightInfo
{
    vec3 position;
} light;

layout(location = 0) in vec4 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec3 outFragPos;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 outUV;
layout(location = 3) out vec3 outLightPos;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() 
{
    //fragment position is in view space
    vec4 vertPos4 = mvp.view * mvp.model * inPos;
    outFragPos = vec3(vertPos4) / vertPos4.w;

    //transform the light position from world space to view space
    vec4 lightPos4 = mvp.view * vec4(light.position, 1.0);
    outLightPos = vec3(lightPos4) / lightPos4.w;
    
    //transpose/inverse expensive, but useful for demo purposes
    outNormal = vec3(mvp.normalMat * vec4(inNormal, 0.0f));    
    
    //pass-through texture coordinates
    outUV = inUV;



    gl_Position = mvp.projection * mvp.view * mvp.model * inPos;
}