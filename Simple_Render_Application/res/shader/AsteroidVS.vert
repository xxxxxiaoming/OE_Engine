#version 460 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec2 VertexTexCoord;
layout (location = 5) in mat4 Model;

out vec2 TexCoord;

uniform mat4 u_View;
uniform mat4 u_Projection;

void main() {
    gl_Position = u_Projection * u_View * Model * vec4(VertexPosition, 1.0);
    TexCoord = VertexTexCoord;
}