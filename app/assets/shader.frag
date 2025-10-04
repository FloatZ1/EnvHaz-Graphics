
#version 460 core
#extension GL_ARB_bindless_texture : require

in vec2 TexCoords;
in uint MatID;

struct Material {
    uvec2 albedo; // 64-bit texture handle
    uvec2 prm;
    uvec2 normalMap;
    uvec2 emission;

    float luminance;
};

layout(binding = 3, std430) readonly buffer ssbo3 {
    Material materials[];
};

out vec4 FragColor;

void main() {
    // Reconstruct bindless sampler2D from handle
    sampler2D albedoTex = sampler2D(materials[MatID].albedo);

    vec4 albedoColor = texture(albedoTex, TexCoords);

    FragColor = vec4(1.0, 1.0, 1.0, 1.0); //albedoColor;
}
