#pragma once

// language=GLSL
static const char pbrVsCode[] = R"glsl(
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 3) in vec3 normal;
layout(location = 5) in vec4 tangent;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;
uniform vec3 u_CameraPosition;	// 相机世界坐标
uniform mat3 u_NormalMat;       // 法线变换矩阵(模型变换矩阵左上角3x3的逆矩阵的转置矩阵)

out vec2 v_TexCoord;
out vec3 v_LookAtCamera;
out vec3 v_FragPosition;
out vec4 v_T;
out vec3 v_N;

void main() {
	gl_Position = u_Projection * u_View * u_Model * vec4(position, 1.0f);
	
	vec4 worldPosition = u_Model * vec4(position, 1.0);
	v_TexCoord = texCoord;
	v_LookAtCamera = u_CameraPosition.xyz - worldPosition.xyz;
	v_FragPosition = vec3(worldPosition);
    
	mat3 model3 = mat3(u_Model);
	v_T.xyz = model3 * tangent.xyz;
	v_T.w = tangent.w;
	v_N = normalize(u_NormalMat * normal);
}
)glsl";

// language=GLSL
static const char skyBoxVS[] = R"glsl(
#version 460 core

layout(location = 0) in vec3 position;

uniform mat4 u_View;
uniform mat4 u_Projection;

out vec3 v_TexCoord;

void main() {
	v_TexCoord = position;
	vec4 projPos = u_Projection * u_View * vec4(position, 1.0);
	gl_Position = projPos.xyww;
}
)glsl";