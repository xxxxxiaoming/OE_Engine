#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in float textureSlot;
layout(location = 3) in vec3 normal;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;
uniform vec3 u_CameraPosition;	// 相机世界坐标
uniform vec3 u_LightPosition;   // 光源世界坐标
uniform mat3 u_NormalMat;       // 法线变换矩阵(模型变换矩阵左上角3x3的逆矩阵的转置矩阵)

out vec2 v_TexCoord;
out float v_TexSlot;
out vec3 v_Normal;
out vec3 v_LookAtLight;
out vec3 v_LookAtCamera;

void main() {
	gl_Position = u_Projection * u_View * u_Model * position;
	vec4 worldPosition = u_Model * position;
	v_TexCoord = texCoord;
	v_TexSlot = textureSlot;
	v_Normal = normalize(u_NormalMat * normal);
	v_LookAtLight = normalize(u_LightPosition.xyz - worldPosition.xyz);
	v_LookAtCamera = normalize(u_CameraPosition.xyz - worldPosition.xyz);
};