#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(triangles, equal_spacing) in;

layout(binding = 1) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 proj;
	float dispMod;
} ubo;

layout(binding = 2)  uniform sampler2D texDisplacement;

layout (location = 0) in vec3 inColor[];
layout (location = 1) in vec2 inUV[];

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 outUV;

void main()
{
    vec4 pos = (gl_TessCoord.x * gl_in[0].gl_Position) +
                  (gl_TessCoord.y * gl_in[1].gl_Position) +
                  (gl_TessCoord.z * gl_in[2].gl_Position);
	
	outUV = gl_TessCoord.x * inUV[0] +
			gl_TessCoord.y * inUV[1] +
			gl_TessCoord.z * inUV[2];

	float displacement = texture(texDisplacement, outUV).x;
	pos.z = displacement * ubo.dispMod;

	gl_Position = ubo.proj * ubo.view * ubo.model * pos;

	

	//average the colors
	float d1 = distance(gl_TessCoord, gl_in[0].gl_Position.xyz);
	float d2 = distance(gl_TessCoord, gl_in[1].gl_Position.xyz);
	float d3 = distance(gl_TessCoord, gl_in[2].gl_Position.xyz);

	vec3 sumWeightedColors = d1 * inColor[0] + d2 * inColor[1] + d3 * inColor[2];
	outColor = sumWeightedColors;//normalize(sumWeightedColors);
	//outColor = gl_TessCoord.x * inColor[0] + 
		//	   gl_TessCoord.y * inColor[1] +
			//   gl_TessCoord.z * inColor[2];


}