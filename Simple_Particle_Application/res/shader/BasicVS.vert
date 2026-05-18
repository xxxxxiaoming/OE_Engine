#version 330 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec2 VertexTexCoord;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec2 TexCoord;

void main() {
    gl_Position = u_Projection * u_View * u_Model * vec4(VertexPosition.xyz, 1.0);
    TexCoord = VertexTexCoord; 
}