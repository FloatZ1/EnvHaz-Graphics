
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
};

out uint MatID;

layout(binding = 0, std430) readonly buffer ssbo0 {
    InstanceData data[];
};

void main()
{
    MatID = data[gl_InstanceID].materialID;
    TexCoords = aTexCoords;
    gl_Position = camMats.projection * camMats.view * data[gl_InstanceID].model * vec4(aPos, 1.0);
}
