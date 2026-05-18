#version 330 core

layout (points) in;
layout (triangle_strip, max_vertices = 5) out;

in vec4 v_VertexColor[1];
out vec4 v_FragColor;

void build_house(vec4 position)
{
    v_FragColor = v_VertexColor[0];
    
    gl_Position = position + vec4(-0.2, -0.2, 0.0, 0.0);    // 1:左下
    EmitVertex();
    gl_Position = position + vec4( 0.2, -0.2, 0.0, 0.0);    // 2:右下
    EmitVertex();
    gl_Position = position + vec4(-0.2,  0.2, 0.0, 0.0);    // 3:左上
    EmitVertex();
    gl_Position = position + vec4( 0.2,  0.2, 0.0, 0.0);    // 4:右上
    EmitVertex();
    gl_Position = position + vec4( 0.0,  0.4, 0.0, 0.0);    // 5:顶部
    EmitVertex();
    EndPrimitive();
}

void main() {
    vec4 position = gl_in[0].gl_Position;
    build_house(position);
}