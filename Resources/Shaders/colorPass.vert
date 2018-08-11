#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform Matrices 
{
    mat4 model;
    mat4 view;
    mat4 projection;
    mat4 normalMat;
} mvp;

layout(binding = 1) uniform UBO
{
    mat4 shadowMatrix;
};

layout(location = 0) in vec4 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec2 inUV;

layout(location = 0) out vec3 outWorldPosition;
layout(location = 1) out vec3 outViewNormal;
layout(location = 2) out vec3 outEyePosition;
layout(location = 3) out vec4 outShadowCoordinate;

void main()
{
    vec4 worldPos = mvp.model * inPos;
    vec4 eyeViewPos = mvp.view * worldPos;
    vec4 clipPos = mvp.projection * eyeViewPos;

    outWorldPosition = worldPos.xyz;
    outEyePosition = eyeViewPos.xyz;
    outShadowCoordinate = shadowMatrix * worldPos;
    outViewNormal = mat3(mvp.view * mvp.model) * inNormal;

    gl_Position = clipPos;
}