#ifndef ENVHAZGRAPHICS_HDR_SHADER_SOURCE_HPP
#define ENVHAZGRAPHICS_HDR_SHADER_SOURCE_HPP

#include <string>
static const std::string g_strHDRVertexSourceCode = R"glsl(
     //@@start@@ HDR ScreenRenderVS shader @@end@@
       #version 460 core

const vec2 verts[3] = vec2[](
    vec2(-1.0, -1.0),
    vec2( 3.0, -1.0),
    vec2(-1.0,  3.0)
);

out vec2 TexCoords;

void main()
{
    TexCoords = verts[gl_VertexID] * 0.5 + 0.5;
    gl_Position = vec4(verts[gl_VertexID], 0.0, 1.0);
}
)glsl";

static const std::string g_strHDRFragmentSourceCode = R"glsl(

     //@@start@@ HDR ScreenRenderFS shader @@end@@
     #version 460 core

layout(binding = 0) uniform sampler2D gAlbedo;
layout(binding = 1) uniform sampler2D gNormal;
layout(binding = 2) uniform sampler2D gPRM;
layout(binding = 3) uniform sampler2D gEmission;
layout(binding = 4) uniform sampler2D gDepth;

in vec2 TexCoords;
out vec4 FragColor;

void main()
{
    vec3 albedo   = texture(gAlbedo, TexCoords).rgb;
    vec3 emission = texture(gEmission, TexCoords).rgb;

    // Basic HDR test output
    vec3 hdrColor = albedo + emission;

    FragColor = vec4(hdrColor, 1.0);
}



)glsl";

static const std::string g_strToneVertexSourceCode = R"glsl(
     //@@start@@ Tone ScreenRenderVS shader @@end@@
     #version 460 core

const vec2 verts[3] = vec2[](
    vec2(-1.0, -1.0),
    vec2( 3.0, -1.0),
    vec2(-1.0,  3.0)
);

out vec2 TexCoords;

void main()
{
    TexCoords = verts[gl_VertexID] * 0.5 + 0.5;
    gl_Position = vec4(verts[gl_VertexID], 0.0, 1.0);
}

)glsl";

static const std::string g_strToneFragmentSourceCode = R"glsl(

     //@@start@@ Tone ScreenRenderFS shader @@end@@
#version 460 core

layout(binding = 0) uniform sampler2D u_HDRColor;

in vec2 TexCoords;
out vec4 FragColor;

uniform float exposure = 1.0;

void main()
{
    vec3 hdrColor = texture(u_HDRColor, TexCoords).rgb;

    // Reinhard tone mapping
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));

    // Optional exposure
    mapped = vec3(1.0) - exp(-hdrColor * exposure);

    // Gamma correction
    mapped = pow(mapped, vec3(1.0 / 2.2));

    FragColor = vec4(mapped, 1.0);
}



)glsl";

#endif
