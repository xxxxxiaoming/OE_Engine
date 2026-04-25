#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec4 v_Color;

void main()
{
	gl_Position = u_Projection * u_View * u_Model * position;
	v_Color = color;
}