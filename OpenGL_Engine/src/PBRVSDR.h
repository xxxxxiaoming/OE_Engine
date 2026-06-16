#pragma once

// language=glsl
static const char pbrVsDRCode[] = R"glsl(
#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;
layout (location = 3) in vec3 normal;
layout (location = 5) in vec4 tangent;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;
uniform mat3 u_NormalMat;
uniform mat4 u_LightSpace;

out vec2 v_TexCoord;
out vec3 v_FragPosition;
out vec4 v_LightSpacePosition;
out vec4 v_T;
out vec3 v_N;

void main()
{
    gl_Position = u_Projection * u_View * u_Model * vec4(position, 1.0);

    vec4 worldPosition = u_Model * vec4(position, 1.0);
    mat3 model3 = mat3(u_Model);

    v_T.xyz = model3 * tangent.xyz;
    v_T.w = tangent.w;
    v_N = u_NormalMat * normal;

    v_TexCoord = texCoord;
    v_FragPosition = worldPosition.xyz;
    v_LightSpacePosition = u_LightSpace * u_Model * vec4(position, 1.0);
}
)glsl";

// language=GLSL
static const char pbrVsLightCode[] = R"glsl(
#version 460 core

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec2 texCoord;

out vec2 v_TexCoord;

void main(){
    gl_Position = vec4(vertexPosition.x, vertexPosition.y, 0.0, 1.0);
    v_TexCoord = texCoord;
}
)glsl";