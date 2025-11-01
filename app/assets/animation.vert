#version 460 core
#extension GL_ARB_bindless_texture : require

// Vertex attributes
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in ivec4 aBoneIDs;
layout(location = 4) in vec4 aBoneWeights;

// Outputs to fragment shader
out vec2 TexCoords;
out vec3 FragNormal;
flat out uint MatID;

// Camera matrices (from SSBO)
struct VP {
    mat4 view;
    mat4 projection;
};
layout(std430, binding = 5) readonly buffer ssbo5 {
    VP camMats;
};

// Instance data
struct InstanceData {
    mat4 model;
    uint materialID;
    uint modelMatID; // unused
};
layout(std430, binding = 0) readonly buffer ssbo0 {
    InstanceData data[];
};

// Joint matrices (skinning)
layout(std430, binding = 2) readonly buffer ssbo8 {
    mat4 jointMatrices[];
};

void main()
{
    // Instance index
    uint curID = gl_DrawID + gl_InstanceID;

    // Pass material ID to fragment shader
    MatID = data[curID].materialID;
    TexCoords = aTexCoords;

    // ==== Skinning computation ====
    // Accumulate weighted transforms
    mat4 skinMat = mat4(0.0);
    skinMat += jointMatrices[aBoneIDs.x] * aBoneWeights.x;
    skinMat += jointMatrices[aBoneIDs.y] * aBoneWeights.y;
    skinMat += jointMatrices[aBoneIDs.z] * aBoneWeights.z;
    skinMat += jointMatrices[aBoneIDs.w] * aBoneWeights.w;

    // Apply to vertex position and normal
    vec4 skinnedPos = skinMat * vec4(aPos, 1.0);
    vec3 skinnedNormal = normalize(mat3(skinMat) * aNormal);

    // ==== Final position ====
    mat4 model = data[curID].model;
    vec4 worldPos = model * skinnedPos;
    FragNormal = normalize(mat3(model) * skinnedNormal);

    gl_Position = camMats.projection * camMats.view * worldPos;
}
