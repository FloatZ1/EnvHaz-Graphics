#version 460 core

//Red Channel: Roughness (determines how blurry or sharp light reflections are).

//Green Channel: Metalness (defines if a surface is dielectric/plastic or metallic).

//Blue Channel: Often used for Ambient Occlusion (AO) or Specular intensity, depending on the specific shader setup.

//Alpha Channel: Sometimes contains a cavity map or additional glossiness data.

layout(binding = 0) uniform sampler2D gAlbedo;
layout(binding = 1) uniform sampler2D gNormal;
layout(binding = 2) uniform sampler2D gPRM;
layout(binding = 3) uniform sampler2D gEmission;
layout(binding = 4) uniform sampler2D gDepth;

struct VP {
    mat4 view;
    mat4 projection;
};

layout(std430, binding = 5) readonly buffer ssbo5 {
    VP camMats;
};

in vec2 TexCoords;
out vec4 FragColor;

uniform vec3 camPos;

#define PI 3.14159265359
#define EPSILON 0.0001

vec3 GetDiffuse() {
    return texture(gAlbedo, TexCoords).rgb / PI;
}

vec3 GetN() {
    vec3 sampleN = texture(gNormal, TexCoords).rgb;
    vec3 normal = sampleN * 2.0 - 1.0;

    return normalize(normal);
}
vec3 GetH(vec3 L, vec3 V) {
    return normalize(L + V);
}

float Distribution(float a, float NoH) {
    float roughsqrt = a * a;
    float a2 = roughsqrt * roughsqrt;
    float denominator = PI * pow((pow(NoH, 2.0) * (a2 - 1.0) + 1.0), 2.0);

    return max(a2 / denominator, EPSILON);
}

float GeometrySchlickGGX(float NoV, float k) {
    float nominator = NoV;
    float denominator = NoV * (1.0 - k) + k;

    return nominator / denominator;
}

float Geometry(float NoV, float NoL, float k) {
    float ggx1 = GeometrySchlickGGX(NoV, k);
    float ggx2 = GeometrySchlickGGX(NoL, k);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 BRDF_Cook_Torance(vec3 F0, float NoV, float NoL, float NoH, float VoH, vec3 albedo,
    float roughness, float metalness) {
    vec3 diffuse = albedo / PI;

    vec3 F = fresnelSchlick(VoH, F0);
    float D = Distribution((roughness), NoH);
    float G = Geometry(NoV, NoL, (pow(roughness + 1.0, 2.0) / 8.0));

    vec3 Ks = F;
    vec3 Kd = (vec3(1.0) - Ks) * (1.0 - metalness);

    vec3 nominator = D * G * F;
    float denominator = max(4.0 * (NoV) * (NoL), EPSILON);

    vec3 specular = nominator / denominator;

    return (Kd * diffuse + specular);
}

vec3 GetWorldPosition(vec2 UV) {
    float z = texture(gDepth, UV).r;

    vec4 clipPos = vec4(UV * 2.0 - 1.0, z * 2.0 - 1.0, 1.0);

    vec4 worldPos = inverse(camMats.projection * camMats.view) * clipPos;

    return worldPos.xyz / worldPos.w;
}

void main()
{
    vec3 N = GetN();
    vec3 worldPos = GetWorldPosition(TexCoords);
    vec3 V = (camPos - worldPos);

    vec3 lightPos = vec3(-3.0, -3.0, 5.0);
    vec3 L = normalize(lightPos - worldPos); // Light Vector (Surface to Light)
    vec3 H = normalize(V + L);

    float dotNV = max(dot(N, V), 0.0);
    float dotNL = max(dot(N, L), 0.0);
    float dotNH = max(dot(N, H), 0.0);
    float dotVH = max(dot(V, H), 0.0);

    vec3 albedo = texture(gAlbedo, TexCoords).rgb;
    vec3 emission = texture(gEmission, TexCoords).rgb;
    float roughness = texture(gPRM, TexCoords).g;
    float metallic = texture(gPRM.TexCoords).r;

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // Basic HDR test output
    vec3 hdrColor = BRDF_Cook_Torance(F0, dotNV, dotNL, dotNH, dotVH, albedo, roughness, metallic) * dotNL + (emission);

    //gamma correction
    vec3 hdrColor = pow(hdrColor, vec3(1.0 / 2.2));

    FragColor = vec4(hdrColor, 1.0);
}
