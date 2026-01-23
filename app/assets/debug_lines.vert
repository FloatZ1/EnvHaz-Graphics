
#version 460 core

layout(binding = 8, std430) readonly buffer VertexIndexSSBO {
    float data[];
};

struct VP {
    mat4 view;
    mat4 projection;
};

struct DebugInstance {
    mat4 model;
    vec4 color;
};

layout(binding = 9, std430) readonly buffer InstanceSSBO {
    DebugInstance instances[];
};

layout(std140, binding = 5) uniform Camera {
    VP camMats;
};

out vec4 vColor;

void main()
{
    uint instanceIndex = gl_BaseInstance + gl_InstanceID;

    DebugInstance inst = instances[instanceIndex];
    vColor = inst.color;

    uint vid = uint(gl_VertexID + gl_BaseVertex);

    vec3 pos = vec3(
            data[vid * 3 + 0],
            data[vid * 3 + 1],
            data[vid * 3 + 2]
        );

    gl_Position = camMats.projection * camMats.view * inst.model * vec4(pos, 1.0);
}
