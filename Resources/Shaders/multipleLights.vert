#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 0) uniform Matrices 
{
    mat4 model;
    mat4 view;
    mat4 projection;
    mat4 normalMat;
} mvp;

layout(location = 0) in vec4 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec2 inUV;

layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec4 outColor;
layout(location = 2) out vec2 outUV;
layout(location = 3) out mat3 outWorldTBN;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() 
{
    outWorldPos = vec3(mvp.model * inPos);

    vec3 T = mat3(transpose(inverse(mvp.model))) * inTangent;
    vec3 N = mat3(transpose(inverse(mvp.model))) * inNormal;
    vec3 B = cross(N, T);
    outWorldTBN = mat3(T, B, N);

    //pass-through texture coordinates & color
    outUV = inUV;
    outColor = vec4(inColor, 1.0);
    
    gl_Position = mvp.projection * mvp.view * mvp.model * inPos;
}