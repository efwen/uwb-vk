#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 0) uniform Matrices 
{
    mat4 projection;
    mat4 model;
    mat4 view;
    mat4 normalMat;
} mvp;

layout(location = 0) in vec4 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec4 outColor;
layout(location = 2) out vec2 outUV;
layout(location = 3) out vec3 outWorldNormal;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() 
{
    outWorldPos = vec3(mvp.model * inPos);
    mat3 mNormal = transpose(inverse(mat3(mvp.model)));
    //outNormal = normalize(vec3(mvp.normalMat * vec4(inNormal, 0.0)));
    outWorldNormal = mNormal * normalize(inNormal);

    //pass-through texture coordinates & color
    outUV = inUV;
    outColor = vec4(inColor, 1.0);
    
    gl_Position = mvp.projection * mvp.view * mvp.model * inPos;
}