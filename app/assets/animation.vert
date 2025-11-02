#version 460 core
#extension GL_ARB_bindless_texture : require

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in ivec4 aBoneIDs;
layout(location = 4) in vec4 aBoneWeights;

out vec2 TexCoords;
out vec3 FragNormal;
flat out uint MatID;

struct VP {
    mat4 view;
    mat4 projection;
};

layout(std430, binding = 5) readonly buffer ssbo5 {
    VP camMats;
};

struct InstanceData {
    mat4 model;
    uint materialID;
    uint modelMatID;
};
layout(std430, binding = 0) readonly buffer ssbo0 {
    InstanceData data[];
};

// Skinning matrices
layout(std430, binding = 2) readonly buffer ssbo8 {
    mat4 jointMatrices[];
};

void main()
{
    uint curID = gl_DrawID + gl_InstanceID;
    MatID = data[curID].materialID;
    TexCoords = aTexCoords;

    // ✅ Safe skinning matrix initialization
    mat4 skinMat = mat4(0.0);

    // ✅ Only apply valid bone influences (>0 weight && valid joint index)
    if (aBoneWeights.x > 0.0) skinMat += jointMatrices[aBoneIDs.x] * aBoneWeights.x;
    if (aBoneWeights.y > 0.0) skinMat += jointMatrices[aBoneIDs.y] * aBoneWeights.y;
    if (aBoneWeights.z > 0.0) skinMat += jointMatrices[aBoneIDs.z] * aBoneWeights.z;
    if (aBoneWeights.w > 0.0) skinMat += jointMatrices[aBoneIDs.w] * aBoneWeights.w;

    // ✅ Prevent uninitialized matrix if no weights
    if (skinMat == mat4(0.0)) {
        skinMat = mat4(1.0);
    }

    // Skin vertex position & normal
    vec4 skinnedPos = skinMat * vec4(aPos, 1.0);
    vec3 skinnedNormal = normalize(mat3(skinMat) * aNormal);

    // Apply model transform
    mat4 model = data[curID].model;
    vec4 worldPos = model * skinnedPos;
    FragNormal = normalize(mat3(model) * skinnedNormal);

    gl_Position = camMats.projection * camMats.view * worldPos;
}
