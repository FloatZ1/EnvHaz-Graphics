
//@@start@@ ScreenRenderFS shader @@end@@

#version 460 core

layout(binding = 0) uniform sampler2D u_ScreenTex;

out vec4 FragColor;

void main()
{
    vec2 uv = gl_FragCoord.xy / vec2(textureSize(u_ScreenTex, 0));
    FragColor = texture(u_ScreenTex, uv);
}
