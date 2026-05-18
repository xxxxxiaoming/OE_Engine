#version 330 core

uniform sampler2D u_Diffuse;

in vec2 TexCoord;
out vec4 Color;

void main() {
    Color = vec4(texture(u_Diffuse, TexCoord).rgb, 1.0);
}