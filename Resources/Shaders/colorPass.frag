#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inWorldPosition;
layout(location = 1) in vec3 inViewNormal;
layout(location = 2) in vec3 inEyePosition;
layout(location = 3) in vec4 inShadowCoordinate;


layout(location = 0) out vec4 outFragColor;
void main()
{
    outFragColor = vec4(1.0);
}