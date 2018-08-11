#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable


layout(binding = 0) uniform MVP {
    mat4 model;
    mat4 view;
    mat4 projection;
    mat4 normalMat;
} mvp;

layout(location = 0) in vec4 inPos;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() 
{
    gl_Position = mvp.projection * mvp.view * mvp.model * inPos;
}