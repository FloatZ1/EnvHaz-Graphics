
#version 460 core

layout(location = 0) in vec3 aPos;

// Instance data
struct DebugInstance {
    mat4 model;
    vec4 color;
};
layout(std430, binding = 10) readonly buffer InstanceSSBO {
    DebugInstance instances[];
};

// Camera uniforms
struct VP {
    mat4 view;
    mat4 projection;
};
layout(std430, binding = 5) readonly buffer Camera {
    VP camMats;
};

// Output color to fragment shader
out vec4 vColor;

void main()
{
    // Get correct instance
    uint instanceIndex = gl_BaseInstance + gl_InstanceID;
    vColor = instances[instanceIndex].color;

    // Transform vertex by model, view, projection
    gl_Position =
        camMats.projection *
            camMats.view *
            instances[instanceIndex].model *
            vec4(aPos, 1.0);
}
