#pragma once

// language=GLSL
static const char fsSSAOCode[] = R"glsl(
#version 460 core

uniform sampler2D u_PositionMap;
uniform sampler2D u_NormalMap;
uniform sampler2D u_NoiseMap;
uniform vec3 u_Samples[64];
uniform mat4 u_View;
uniform mat4 u_Projection;
uniform int u_Resolution[2];

in vec2 v_TexCoord;
out vec4 color;

const int kernelSize = 64;
const float radius = 25;
const float bias = 0.5;

void main()
{
    float occlusion = 0.0;
    
    vec2 noiseScale = vec2(u_Resolution[0] / 4.0, u_Resolution[1] / 4.0);
    
    vec4 fragPosition = vec4(texture(u_PositionMap, v_TexCoord).rgb, 1.0);
    vec4 fragViewPosition = u_View * fragPosition;
    vec3 normal = texture(u_NormalMap, v_TexCoord).rgb;
    vec3 noise = texture(u_NoiseMap, v_TexCoord * noiseScale).rgb;

    vec3 tangent = normalize(noise - normal * dot(noise, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    
    for(int count = 0; count < kernelSize; count++)
    {
        vec3 sampleVec = TBN * u_Samples[count].xyz;
        vec4 sampleWorldPosition = vec4(fragPosition.xyz + sampleVec * radius, 1.0);
        vec4 sampleViewPosition = u_View * sampleWorldPosition;
        
        vec4 sampleClipPosition = u_Projection * sampleViewPosition;
        sampleClipPosition.xyz = sampleClipPosition.xyz / sampleClipPosition.w; //NDC
        sampleClipPosition.xy = sampleClipPosition.xy * 0.5 + 0.5;
        
        vec4 geoWorldPosition = vec4(texture(u_PositionMap, sampleClipPosition.xy).rgb, 1.0);
        float geoDepth = (u_View * geoWorldPosition).z;
    
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragViewPosition.z - geoDepth));
        occlusion += ((geoDepth >= sampleViewPosition.z + bias) ? 1.0 : 0.0) * rangeCheck;
    }

    occlusion = occlusion / kernelSize;
    occlusion = 1 - occlusion;

    color = vec4(occlusion, occlusion, occlusion, 1.0);
}

)glsl";

// language=GLSL
static const char fsSSAOBlurCode[] = R"glsl(
#version 460 core

uniform sampler2D u_SSAOMap;

in vec2 v_TexCoord;
out vec4 color;

void main() 
{
    vec2 texelSize = 1.0 / vec2(textureSize(u_SSAOMap, 0));
    
    float result = 0.0;
    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(u_SSAOMap, v_TexCoord + offset).r;
        }
    }

    result /= (4.0 * 4.0);

    color = vec4(result, result, result, 1.0);
}  
)glsl";