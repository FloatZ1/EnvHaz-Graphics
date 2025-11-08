
#version 460 core

#extension GL_ARB_bindless_texture : require

layout(location = 0) in vec3 aPos;
layout(location = 2) in vec3 aNormal;
layout(location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

struct VP {
    mat4 view;
    mat4 projection;
};

layout(binding = 5, std430) readonly buffer ssbo5 {
    VP camMats;
};

struct InstanceData {
    mat4 model;
    uint materialID;
    uint modelMatID;
};

flat out uint MatID;

layout(binding = 0, std430) readonly buffer ssbo0 {
    InstanceData data[];
};

layout(binding = 7, std430) readonly buffer ssbo7 {
    mat4 modelMat[];
};

layout(binding = 2, std430) readonly buffer ssbo8 {
    mat4 jointMatrices[];
}

void main()
{
uint curID = gl_DrawID + gl_InstanceID;
MatID = data[curID] . materialID;
uint partMat = data[curID].modelMatID;
TexCoords = aTexCoords;

gl_Position = camMats . projection * camMats . view * data[curID] . model * modelMat[partMat] * vec4(aPos, 1.0);

}
