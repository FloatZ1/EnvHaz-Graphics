#version 460 core
#extension GL_ARB_bindless_texture : require

// ====================================================================
// Vertex Attributes (Input from VBO)
// ====================================================================
layout(location = 0) in vec3 aPos; // Vertex Position
layout(location = 1) in vec2 aTexCoords; // Texture Coordinates
layout(location = 2) in vec3 aNormal; // Vertex Normal
layout(location = 3) in ivec4 aBoneIDs; // Bone Indices (up to 4)
layout(location = 4) in vec4 aBoneWeights; // Bone Weights (up to 4)

// ====================================================================
// Outputs to Fragment Shader (Interpolated)
// ====================================================================
out vec2 TexCoords;
out vec3 FragNormal;
flat out uint MatID; // Flat means no interpolation (required for IDs)

// ====================================================================
// Uniform Buffer Objects (Camera Data)
// ====================================================================
struct VP {
    mat4 view;
    mat4 projection;
};
// Binding 5: Camera View and Projection matrices
layout(std430, binding = 5) readonly buffer ssbo5 {
    VP camMats;
};

// ====================================================================
// Shader Storage Buffer Objects (Per-Instance Data)
// ====================================================================
struct InstanceData {
    mat4 model; // Model-to-World matrix
    uint materialID;
    uint modelMatID; // unused
};
// Binding 0: Array of Instance Data
layout(std430, binding = 0) readonly buffer ssbo0 {
    InstanceData data[];
};

// ====================================================================
// Shader Storage Buffer Objects (Skinning Data)
// ====================================================================
// Binding 2: Array of final joint matrices (Bone Transforms)
// NOTE: This SSBO is updated by the AnimatedModelManager::UploadBonesToGPU
layout(std430, binding = 2) readonly buffer ssbo8 {
    mat4 jointMatrices[];
};

// ====================================================================
// Main Function
// ====================================================================
void main()
{
    // Combine draw index and instance index to get the global index for instance data
    uint curID = gl_DrawID + gl_InstanceID;

    // --- Prepare Fragment Outputs ---
    MatID = data[curID].materialID;
    TexCoords = aTexCoords;

    // --- Bone Skinning Computation ---

    // Accumulate the weighted bone transforms into a single matrix
    mat4 skinMat = mat4(0.0);

    // Check if the vertex has weight, if not, treat as identity (optional)
    if (dot(aBoneWeights, vec4(1.0)) > 0.0) {
        skinMat += jointMatrices[aBoneIDs.x] * aBoneWeights.x;
        skinMat += jointMatrices[aBoneIDs.y] * aBoneWeights.y;
        skinMat += jointMatrices[aBoneIDs.z] * aBoneWeights.z;
        skinMat += jointMatrices[aBoneIDs.w] * aBoneWeights.w;
    } else {
        // If no weights, use an identity matrix (no skinning)
        skinMat = mat4(1.0);
    }

    // Apply the skinning matrix to the local vertex position
    vec4 skinnedPos = skinMat * vec4(aPos, 1.0);
    // Apply the skinning matrix to the local normal (using mat3 for normals)
    vec3 skinnedNormal = normalize(mat3(skinMat) * aNormal);

    // --- World Space Transformation ---
    mat4 model = data[curID].model;

    // Transform the skinned position to world space
    vec4 worldPos = model * skinnedPos;

    // Transform the skinned normal to world space (normalize after model transform)
    FragNormal = normalize(mat3(model) * skinnedNormal);

    // --- Final Clip Space Position ---
    gl_Position = camMats.projection * camMats.view * worldPos;
}
