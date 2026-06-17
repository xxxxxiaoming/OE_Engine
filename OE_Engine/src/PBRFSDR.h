#pragma once

// language=glsl
static const char pbrFsDRCode[] = R"glsl(
#version 460 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gLightSpacePosition;
layout (location = 2) out vec3 gN;
layout (location = 3) out vec4 gAlbedo;
layout (location = 4) out vec4 gMetallicRoughnessAO;
layout (location = 5) out vec4 gEmissive;

struct Material {
    sampler2D albedo;
    sampler2D normal;
    sampler2D metallic;
    sampler2D roughness;
    sampler2D AO;
    sampler2D emissive;

    float metallicFactor;
    float roughnessFactor;
    float cutOff;
    vec4 albedoFactor;
};

uniform Material u_Material;
uniform bool u_UseMRA;
uniform bool u_UseMR;

in vec2 v_TexCoord;
in vec3 v_FragPosition;
in vec4 v_LightSpacePosition;
in vec4 v_T;
in vec3 v_N;

mat3 correctionTBN(vec3 v_T, vec3 v_N, float handedness)
{
    vec3 T = normalize(v_T);
    vec3 B;
    vec3 N = normalize(v_N);

    T = normalize(T - dot(T,N)*N);

    B = cross(N, T) * handedness;

    mat3 TBN = mat3(T,B,N);

    return TBN;
}

void main()
{
    float alpha = texture(u_Material.albedo, v_TexCoord).a;
    if (alpha < u_Material.cutOff)
        discard;

    gPosition.rgb = v_FragPosition.xyz;
    gLightSpacePosition.rgb = v_LightSpacePosition.xyz;

    mat3 TBN = correctionTBN(v_T.xyz, v_N, v_T.w);
    vec3 normalVec = texture(u_Material.normal, v_TexCoord).rgb;

    normalVec = normalVec * 2 - 1;
    normalVec = TBN * normalVec;
    normalVec = normalize(normalVec);
    //normalVec = normalize(v_N);

    gN = normalVec * 0.5 + 0.5;
    gAlbedo = vec4(pow(texture(u_Material.albedo, v_TexCoord).rgb, vec3(2.2)), 1.0) * u_Material.albedoFactor;
    gEmissive = vec4(pow(texture(u_Material.emissive, v_TexCoord).rgb, vec3(2.2)), 1.0);
    gMetallicRoughnessAO = u_UseMRA ? 
                           vec4(texture(u_Material.metallic, v_TexCoord).b * u_Material.metallicFactor, texture(u_Material.roughness, v_TexCoord).g * u_Material.roughnessFactor, texture(u_Material.AO, v_TexCoord).r, 1.0) 
                           : vec4(texture(u_Material.metallic, v_TexCoord).r * u_Material.metallicFactor, texture(u_Material.roughness, v_TexCoord).r * u_Material.roughnessFactor, texture(u_Material.AO, v_TexCoord).r, 1.0);
}
)glsl";

// Final lighting shader
// language=glsl
static const char pbrFsLightCode[] = R"glsl(
#version 460 core

const int MAX_POINT_OR_SPOT_LIGHT_NUM = 4;
const float PI = 3.14159265359;

struct BRDF {
    vec3 kD;
    vec3 specular;
};

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 color;
};

struct SpotLight {
    vec3 position;
    vec3 direction;

    float innerAngle;
    float outterAngle;

    float constant;
    float linear;
    float quadratic;

    vec3 color;
};

struct DirectionLight {
    vec3 direction;
    vec3 color;
};

struct IBL {
    samplerCube irradiance;
    samplerCube radiance;
    sampler2D BRDF_LUT;
};

struct LightConfig {
    bool enableDirectionalLight;
    bool enableIBL;
    int pointLightNum;
    int spotLightNum;
};

uniform LightConfig u_LightConfig;
uniform DirectionLight u_DirectionLight;
uniform PointLight u_PointLights[MAX_POINT_OR_SPOT_LIGHT_NUM];
uniform SpotLight u_SpotLights[MAX_POINT_OR_SPOT_LIGHT_NUM];
uniform IBL u_IBL;

uniform float u_FarPlane;
uniform vec3 u_CameraPosition;

uniform sampler2D u_PositionMap;
uniform sampler2D u_LightSpacePositionMap;
uniform sampler2D u_NormalMap;
uniform sampler2D u_AlbedoMap;
uniform sampler2D u_EmissiveMap;
uniform sampler2D u_MetallicRoughnessAOMap;
uniform sampler2D u_SSAOMap;
uniform sampler2D u_ShadowDepthMap;
uniform samplerCube u_PointLightsDepthMap[MAX_POINT_OR_SPOT_LIGHT_NUM];

vec3 lookAtCamera_Normalized;
vec3 sampleOffsetDirections[20] = 
{
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
};

in vec2 v_TexCoord;
out vec4 color;

float calcNDF(float roughness, vec3 normal, vec3 halfway)
{
    float alpha = roughness * roughness;
    float aSquare = alpha * alpha;
    float NdotH = max(dot(normal, halfway), 0.0);
    float dotNVSquare = NdotH * NdotH;
    float denom = dotNVSquare * (aSquare - 1) + 1;
    return aSquare / (PI * denom * denom);
}

vec3 calcFresnelSchlick(vec3 halfway, vec3 view, vec3 F0)
{
    float HdotV = max(dot(halfway, view), 0.0);
    return F0 + (vec3(1.0) - F0) * pow(clamp(1.0 - HdotV, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
} 

float calcGeometryGGX(vec3 view, vec3 normal, float roughness)
{
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    float dotVN = max(dot(normal, view), 0.0);
    return dotVN / (dotVN * (1 - k) + k);
}

float calcGeometry(vec3 view, vec3 light, vec3 normal, float roughness)
{
    float G1 = calcGeometryGGX(view, normal, roughness);
    float G2 = calcGeometryGGX(light, normal, roughness);
    return G1 * G2;
}

BRDF calcBRDF(vec3 albedo, float metallic, float roughness, vec3 normal, vec3 view, vec3 lightDirection)
{
    vec3 halfway = normalize(view + lightDirection);

    float N = calcNDF(roughness, normal, halfway);
    float G = calcGeometry(view, lightDirection, normal, roughness);
    vec3 F0 = vec3(0.04);
    
    F0 = mix(F0, albedo, metallic);

    vec3 F = calcFresnelSchlick(halfway, view, F0);
    vec3 ks = F;
    vec3 kd = (vec3(1.0) - F) * (1.0 - metallic);

    vec3 numerator = N * G * F;
    float denominator = 4 * max(dot(view, normal), 0.0) * max(dot(lightDirection, normal), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    BRDF result;
    result.kD = kd;
    result.specular = specular;

    return result;
}

vec3 calcIBLLight(vec3 normal, vec3 view, vec3 albedo, float roughness, float metallic, vec3 F0)
{
    if(!u_LightConfig.enableIBL)
        return vec3(0.03);
    
    vec3 reflectVec = normalize(reflect(-view, normal));
    vec3 irradiance = texture(u_IBL.irradiance, normal).rgb;
    vec3 radiance = textureLod(u_IBL.radiance, reflectVec, roughness * 7.0).rgb;
    vec2 brdf = texture(u_IBL.BRDF_LUT, vec2(max(dot(view, normal), 0.0), roughness)).rg;
    
    vec3 F = fresnelSchlickRoughness(max(dot(normal, view), 0.0), F0, roughness);
    vec3 kD = 1.0 - F;
    kD *= (1.0 - metallic);
    vec3 light = kD * irradiance * albedo + (F * brdf.r + brdf.y) * radiance;

    return light;
}

vec3 calcDirectionLight(vec3 albedo, float metallic, float roughness, vec3 normal, vec3 view)
{
    if(!u_LightConfig.enableDirectionalLight)
        return vec3(0.0, 0.0, 0.0);

    ivec2 depthMapSize = textureSize(u_ShadowDepthMap, 0);
    vec2 texelSize = vec2(1.0 / depthMapSize.x, 1.0 / depthMapSize.y);
    float shadowDebuff = 0.0;

    vec3 shadowMapPositionNDC = texture(u_LightSpacePositionMap, v_TexCoord).rgb * 0.5 + 0.5;
    vec2 shadowMapTexCoord = shadowMapPositionNDC.xy;

    vec3 lightDir = normalize(-u_DirectionLight.direction);
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);

    for(int x = -1; x <= 1; x++)
        for(int y = -1; y <= 1; y++)
        {
    
            float shadowMapDepth = texture(u_ShadowDepthMap, shadowMapTexCoord + vec2(x, y) * texelSize).r;
            shadowDebuff += (shadowMapPositionNDC.z - bias) > shadowMapDepth ? 0.0 : 1.0;
        }

    shadowDebuff /= 9.0;
    shadowDebuff = shadowMapPositionNDC.z > 1.0 ? 1.0 : shadowDebuff;

    BRDF brdf = calcBRDF(albedo, metallic, roughness, normal, view, lightDir);

    vec3 directionLight = (brdf.kD * albedo / PI + brdf.specular) * u_DirectionLight.color * max(dot(normal, lightDir), 0.0);

    return directionLight * shadowDebuff;
}

vec3 calcPointLight(vec3 albedo, float metallic, float roughness, vec3 normal, vec3 view)
{
    if(u_LightConfig.pointLightNum <= 0)
        return vec3(0.0);

    int num = min(u_LightConfig.pointLightNum, MAX_POINT_OR_SPOT_LIGHT_NUM);
    int samples = 20;
    
    float attenuation = 1.0;
    float bias = 0.05;
    float shadowDebuff = 0.0;
    float diskRadius = 0.05;
    
    vec3 result = vec3(0.0);
    vec3 fragPosition = texture(u_PositionMap, v_TexCoord).rgb;

    for(int i = 0; i< num; i++)
    {
        PointLight light = u_PointLights[i];
        float lightDistance = length(light.position - fragPosition);
        vec3 lightDirection = normalize(light.position - fragPosition);
        attenuation = 1.0 / (light.constant + light.linear * lightDistance + light.quadratic * lightDistance * lightDistance);

        for(int j = 0; j < samples; ++j)
        {
            float closestDepth = texture(u_PointLightsDepthMap[i], -lightDirection + sampleOffsetDirections[j] * diskRadius).r;
            closestDepth *= u_FarPlane;
            
            shadowDebuff += ((lightDistance - bias < closestDepth) ? 1.0 : 0.0);
        }

        shadowDebuff /= float(samples);    

        BRDF brdf = calcBRDF(albedo, metallic, roughness, normal, view, lightDirection);
        result += (brdf.kD * albedo / PI + brdf.specular) * light.color * max(dot(normal, lightDirection), 0.0) * shadowDebuff * attenuation;
    }

    return result;
}

vec3 calcSpotLight(vec3 albedo, float metallic, float roughness, vec3 normal, vec3 view)
{
    if(u_LightConfig.spotLightNum <= 0)
        return vec3(0.0);

    float spotFactor = 1.0;
    float attenuation = 1.0;
    int num = min(u_LightConfig.spotLightNum, MAX_POINT_OR_SPOT_LIGHT_NUM);

    vec3 fragPosition = texture(u_PositionMap, v_TexCoord).rgb;
    vec3 lookAtLight = vec3(0.0);
    vec3 result = vec3(0.0);

    for(int i = 0; i < num; i++)
    {
        SpotLight light = u_SpotLights[i];
        lookAtLight = normalize(light.position - fragPosition);

        float lightDistance = length(light.position - fragPosition);
        float temp = dot(normalize(light.direction), normalize(-lookAtLight));

        spotFactor = clamp((temp - cos(light.outterAngle)) / (cos(light.innerAngle) - cos(light.outterAngle)), 0.0, 1.0);
        attenuation = 1.0 / (light.constant + light.linear * lightDistance + light.quadratic * lightDistance * lightDistance);

        BRDF brdf = calcBRDF(albedo, metallic, roughness, normal, view, lookAtLight);
        result += (brdf.kD * albedo / PI + brdf.specular) * light.color * max(dot(normal, lookAtLight), 0.0) * spotFactor * attenuation;
    }

    return result;
}

void main()
{
    vec3 albedo = texture(u_AlbedoMap, v_TexCoord).rgb;
    vec3 emissive = texture(u_EmissiveMap, v_TexCoord).rgb;
    vec3 light = vec3(0.0, 0.0, 0.0);
    
    float metallic = texture(u_MetallicRoughnessAOMap, v_TexCoord).r;
    float roughness = texture(u_MetallicRoughnessAOMap, v_TexCoord).g;
    float ao = texture(u_MetallicRoughnessAOMap, v_TexCoord).b;
    float ssao = texture(u_SSAOMap, v_TexCoord).r;

    vec3 normal = normalize(texture(u_NormalMap, v_TexCoord).rgb * 2.0 - 1.0);
    vec3 view = normalize(u_CameraPosition - texture(u_PositionMap, v_TexCoord).rgb);
    vec3 ambient = calcIBLLight(normal, view, albedo, roughness, metallic, mix(vec3(0.04), albedo, metallic)) * ao * ssao;

    light = calcDirectionLight(albedo, metallic, roughness, normal, view) + calcPointLight(albedo, metallic, roughness, normal, view) + calcSpotLight(albedo, metallic, roughness, normal, view);
    light = light + ambient + emissive * 10.0;
    color = vec4(light.rgb, 1.0);
}

)glsl";