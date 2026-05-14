#version 330 core

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec2 texCoord;

out vec2 v_TexCoord;

void main(){
    gl_Position = vec4(vertexPosition.x, vertexPosition.y, 0.0, 1.0);
    v_TexCoord = texCoord;
}