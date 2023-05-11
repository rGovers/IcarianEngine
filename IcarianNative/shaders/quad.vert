#version 450

layout(location = 0) out vec2 vUV;

void main()
{
    vec2 pos = vec2(gl_VertexIndex % 2, gl_VertexIndex / 2);
    gl_Position = vec4(pos.xy * 2 + -1, 0.0f, 1.0f);
    vUV = pos;
}