#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(triangles, equal_spacing, ccw) in;

layout(binding = 0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout (location = 0) in vec3 inColor[];
layout (location = 1) in vec2 inUV[];

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 outUV;

void main()
{
    gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position) +
                  (gl_TessCoord.y * gl_in[1].gl_Position) +
                  (gl_TessCoord.z * gl_in[2].gl_Position);

	gl_Position = ubo.proj * ubo.view * ubo.model * gl_Position;

	outColor = gl_TessCoord.x * inColor[0] + 
			   gl_TessCoord.y * inColor[1] +
			   gl_TessCoord.z * inColor[2];

	outUV = gl_TessCoord.x * inUV[0] +
			gl_TessCoord.y * inUV[1] +
			gl_TessCoord.z * inUV[2];
}