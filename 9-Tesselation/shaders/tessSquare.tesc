#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(vertices = 3) out;

layout (location = 0) in vec3 inColor[];
layout (location = 1) in vec2 inUV[];

layout (location = 0) out vec3 outColor[3];
layout (location = 1) out vec2 outUV[3];

void main()
{
    if(gl_InvocationID == 0)
    {
        gl_TessLevelInner[0] = 16.0f;

        gl_TessLevelOuter[0] = 16.0f;
        gl_TessLevelOuter[1] = 16.0f;
        gl_TessLevelOuter[2] = 16.0f;
    }
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	outColor[gl_InvocationID] = inColor[gl_InvocationID];
	outUV[gl_InvocationID] = inUV[gl_InvocationID];
}