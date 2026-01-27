
#version 460 core

layout(location = 0) in vec3 aPos;

struct VP {
    mat4 view;
    mat4 projection;
};

struct DebugInstance {
    mat4 model;
    vec4 color;
};

layout(std430, binding = 10) readonly buffer InstanceSSBO {
    DebugInstance instances[];
};

layout(std430, binding = 5) readonly buffer Camera {
    VP camMats;
};

out vec4 vColor;

void main()
{
    uint instanceIndex = gl_BaseInstance + gl_InstanceID;
    DebugInstance inst = instances[instanceIndex];
    vColor = inst.color;

    // World-space line endpoints
    vec3 lineStart = inst.model[3].xyz;
    vec3 lineDir = normalize(inst.model[0].xyz); // X axis = line direction

    // Camera vectors (from view matrix)
    vec3 camRight = vec3(camMats.view[0][0],
            camMats.view[1][0],
            camMats.view[2][0]);

    vec3 camUp = vec3(camMats.view[0][1],
            camMats.view[1][1],
            camMats.view[2][1]);

    // aPos.x = along line [0..1]
    // aPos.y = width [-0.5..0.5]
    float along = aPos.x;
    float width = aPos.y;

    // Position along the line
    vec3 worldPos = lineStart + lineDir * along;

    // Offset quad sideways so it faces the camera
    worldPos += camRight * width;

    gl_Position = camMats.projection * camMats.view * vec4(worldPos, 1.0);
}
