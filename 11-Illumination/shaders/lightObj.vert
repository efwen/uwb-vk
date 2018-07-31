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

layout(location = 0) out vec3 outColor;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() 
{
    gl_Position = mvp.proj * mvp.view * mvp.model * inPos;
    outColor = inColor;
}