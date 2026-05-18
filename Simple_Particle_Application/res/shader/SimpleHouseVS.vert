#version 330 core

layout (location = 0) in vec3 vertexPosition;
layout (location = 4) in vec4 vertexColor;

out vec4 v_VertexColor;

void main() {
    gl_Position = vec4(vertexPosition.xyz, 1.0);
    v_VertexColor = vertexColor;
}