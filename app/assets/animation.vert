
#version 460 core

#extension GL_ARB_bindless_texture : require

// Vertex attributes
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec4 aBoneWeights;
layout(location = 4) in uvec4 aBoneIDs;

// Outputs
out vec2 TexCoords;
out vec3 FragNormal;
flat out uint MatID;

// Camera matrices
struct VP {
    mat4 view;
    mat4 projection;
};
layout(binding = 5, std430) readonly buffer ssbo5 {
    VP camMats;
};

// Instance data
struct InstanceData {
    mat4 model;
    uint materialID;
    uint modelMatID; // unused now
};
layout(binding = 0, std430) readonly buffer ssbo0 {
    InstanceData data[];
};

// Joint matrices for skinning
layout(binding = 2, std430) readonly buffer ssbo8 {
    mat4 jointMatrices[];
};

void main()
{
    uint curID = gl_DrawID + gl_InstanceID;
    MatID = data[curID].materialID;
    TexCoords = aTexCoords;

    // Compute skinned position
    mat4 skinMat = aBoneWeights.x * jointMatrices[aBoneIDs.x] +
            aBoneWeights.y * jointMatrices[aBoneIDs.y] +
            aBoneWeights.z * jointMatrices[aBoneIDs.z] +
            aBoneWeights.w * jointMatrices[aBoneIDs.w];

    vec4 skinnedPos = skinMat * vec4(aPos, 1.0);
    vec3 skinnedNormal = mat3(skinMat) * aNormal; // apply skinning to normal

    FragNormal = normalize(skinnedNormal);
    gl_Position = camMats.projection * camMats.view * data[curID].model * skinnedPos;
}
