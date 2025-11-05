#version 460 core
#extension GL_ARB_bindless_texture : require

// ============================ Vertex Inputs ============================
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in ivec4 aBoneIDs;
layout(location = 4) in vec4 aBoneWeights;

// ============================ Outputs ============================
out vec2 TexCoords;
out vec3 FragNormal;
flat out uint MatID;

// ============================ Camera (UBO/SSBO) ============================
struct VP {
    mat4 view;
    mat4 projection;
};
layout(std430, binding = 5) readonly buffer ssbo5 {
    VP camMats;
};

// ============================ Instance Data ============================
struct InstanceData {
    mat4 model;
    uint materialID;
    uint modelMatID;
};
layout(std430, binding = 0) readonly buffer ssbo0 {
    InstanceData data[];
};

// ============================ Bone Matrices ============================
layout(std430, binding = 2) readonly buffer ssbo2 {
    mat4 jointMatrices[];
};

// ============================ Main ============================
void main()
{
    // Instance selection
    uint curID = gl_DrawID + gl_InstanceID;
    mat4 model = data[curID].model;

    TexCoords = aTexCoords;
    MatID = data[curID].materialID;

    // ----------- Bone Skinning -----------
    vec4 totalPosition = vec4(0.0);
    vec3 totalNormal = vec3(0.0);

    // Sum of weights to detect zero-weight vertices
    float weightSum = aBoneWeights.x + aBoneWeights.y +
            aBoneWeights.z + aBoneWeights.w;

    if (weightSum > 0.0) {
        const int MAX_BONE_INFLUENCE = 4;
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
        {
            int id = aBoneIDs[i];
            float w = aBoneWeights[i];
            if (id < 0 || w <= 0.0)
                continue;

            mat4 boneTransform = jointMatrices[id];
            totalPosition += (boneTransform * vec4(aPos, 1.0)) * w;
            totalNormal += (mat3(boneTransform) * aNormal) * w;
        }
    } else {
        // No bone influence: use model-space position as is
        totalPosition = vec4(aPos, 1.0);
        totalNormal = aNormal;
    }

    // ----------- World Transform -----------
    vec4 worldPos = model * totalPosition;
    FragNormal = normalize(mat3(model) * totalNormal);

    // ----------- Final Clip Position -----------
    gl_Position = camMats.projection * camMats.view * worldPos;
}
