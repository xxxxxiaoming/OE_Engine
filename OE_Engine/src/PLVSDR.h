#pragma once

// GBuffer vertex shader
// language=GLSL
static const char vsShaderCodeDR[] = R"glsl(
#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;
layout (location = 3) in vec3 normal;
layout (location = 5) in vec3 tangent;
layout (location = 6) in vec3 bitangent;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;
uniform mat3 u_NormalMat;
uniform mat4 u_LightSpace;

out vec2 v_TexCoord;
out vec3 v_FragPosition;
out vec4 v_LightSpacePosition;
out vec3 v_T;
out vec3 v_B;
out vec3 v_N;

void main() {
	gl_Position = u_Projection * u_View * u_Model * vec4(position, 1.0f);

	vec4 worldPosition = u_Model * vec4(position, 1.0f);
	mat3 model3 = mat3(u_Model);

	v_T = model3 * tangent;
	v_B = bitangent;	// bitangent 直接透传，因为这个向量会在 fragment shader 中用v_T,v_N重建(correctionTBN)
	v_N = u_NormalMat * normal;

	v_TexCoord = texCoord;
	v_FragPosition = worldPosition.xyz;
	v_LightSpacePosition = u_LightSpace * u_Model * vec4(position, 1.0f);
}
)glsl";

// language=GLSL
static const char vsShaderCodeLight[] = R"glsl(
#version 460 core

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec2 texCoord;

out vec2 v_TexCoord;

void main(){
    gl_Position = vec4(vertexPosition.x, vertexPosition.y, 0.0, 1.0);
    v_TexCoord = texCoord;
}
)glsl";