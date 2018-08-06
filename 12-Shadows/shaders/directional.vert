#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 0) uniform MVP 
{
    mat4 projection;
    mat4 model;
    mat4 view;
    mat4 normalMat;
} mvp;

layout(binding = 1) uniform LightTransform
{
    vec4 position;
    vec4 direction;
} light;


layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;
layout(location = 4) in vec3 inTangent;

layout(location = 0) out vec3 outPos;
layout(location = 1) out vec3 outColor;
layout(location = 2) out vec2 outUV;
layout(location = 3) out mat3 outTBN;
layout(location = 6) out vec4 outLightPosition;
layout(location = 7) out vec4 outLightDirection;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() 
{
    //fragment position is in view space
    outPos = vec3(mvp.view * mvp.model * vec4(inPos, 1.0));

    
    //pass-through texture coordinates
    outUV = inUV;
    outColor = inColor;

    outLightPosition = mvp.view * light.position;
    outLightDirection = mvp.view * light.direction;

    vec3 T = normalize(vec3(mvp.normalMat * vec4(inTangent, 0.0)));
    vec3 N = normalize(vec3(mvp.normalMat * vec4(inNormal, 0.0)));
    vec3 B = cross(N, T);
    outTBN = mat3(T, B, N);

    gl_Position = mvp.projection * mvp.view * mvp.model * vec4(inPos, 1.0);
}