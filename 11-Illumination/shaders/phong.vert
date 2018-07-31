#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable


layout(binding = 0) uniform MVP {
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp;

layout(location = 0) in vec4 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec3 outFragPos;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 outUV;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() 
{
    //fragment position is in view space
    outFragPos = vec3(mvp.view * mvp.model * inPos).xyz;

    //transpose/inverse expensive, but useful for demo purposes
    outNormal = transpose(inverse(mat3(mvp.view * mvp.model))) * inNormal;    
    
    //pass-through texture coordinates
    outUV = inUV;

    gl_Position = mvp.proj * mvp.view * mvp.model * inPos;
}