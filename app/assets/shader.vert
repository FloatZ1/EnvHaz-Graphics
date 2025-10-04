
#version 460 core

#extension GL_ARB_bindless_texture : require

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

layout(binding = 5, std430) readonly buffer ssbo5 {
    mat4 view;
    mat4 projection;
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
    gl_Position = projection * view * data[gl_InstanceID].model * vec4(aPos, 1.0);
}
