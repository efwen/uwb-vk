#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UBO
{
    mat4 MVPMatrix;     //the MVP matrix for the light source
} ubo;

layout(location = 0) in vec4 inPosition;
void main()
{
    gl_Position = ubo.MVPMatrix * inPosition;
}