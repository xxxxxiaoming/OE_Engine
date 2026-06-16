#pragma once

// language=GLSL
static const char pbrFsCode[] = R"glsl(
#version 460 core

const int MAX_POINT_OR_SPOT_LIGHT_NUM = 4;
const float PI = 3.14159265359;

struct BRDF {
    vec3 kD;
    vec3 specular;
};

struct LightCombination {
    vec3 diffuse;
    vec3 specular;
};

struct Material {
    sampler2D albedo;
    sampler2D normal;
    sampler2D metallic;
    sampler2D roughness;
    sampler2D AO;
    sampler2D emissive;
    sampler2D transmission;
    sampler2D background;

    float metallicFactor;
    float roughnessFactor;
    float transmissionFactor;
    float ior;
};

/* 点光源 */
struct PointLight {
    vec3 position;

    float constant;  // 常数衰减系数
    float linear;    // 线性衰减系数
    float quadratic; // 二次衰减系数   

    vec3 color;
};

/* 聚光 */
struct SpotLight {
    vec3 position;
    vec3 direction;
    
    float innerAngle; // angle in radians
    float outterAngle;// angle in radians
    
    float constant;  // 常数衰减系数
    float linear;    // 线性衰减系数
    float quadratic; // 二次衰减系数   

    vec3 color;
};

/* 方向光 */
struct DirectionLight {
    vec3 direction; // 光源方向
    vec3 color;
};

struct IBL {
    samplerCube irradiance;
    samplerCube radiance;
    sampler2D BRDF_LUT;
};


/* 光源配置 */
struct LightConfig {
    bool enableDirectionalLight;  // 方向光的开关
    int spotLightNum;           // 点光源个数，做多支持MAX_POINT_OR_SPOT_NUM这么多个，这个数量没有什么依据，我自己说的
    int pointLightNum;          // 点光源个数，做多支持MAX_POINT_OR_SPOT_NUM这么多个，这个数量没有什么依据，我自己说的
};

uniform IBL u_IBL;
uniform LightConfig u_LightConfig;
uniform DirectionLight u_DirectionLight;
uniform PointLight u_PointLights[MAX_POINT_OR_SPOT_LIGHT_NUM];
uniform SpotLight u_SpotLights[MAX_POINT_OR_SPOT_LIGHT_NUM];
uniform Material u_Material;
uniform bool u_UseMRA;

in vec2 v_TexCoord;
in vec3 v_LookAtCamera;
in vec3 v_FragPosition;
in vec4 v_T;
in vec3 v_N;

out vec4 color;

vec3 lookAtCamera_Normalized;
vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

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

LightCombination calcDirectionLight(vec3 albedo, float metallic, float roughness, vec3 normal, vec3 view)
{
    LightCombination directionLight;
    directionLight.diffuse = vec3(0.0);
    directionLight.specular = vec3(0.0);
    
    if(!u_LightConfig.enableDirectionalLight)
    {
        return directionLight;
    }

    vec3 lightDir = normalize(-u_DirectionLight.direction);

    BRDF brdf = calcBRDF(albedo, metallic, roughness, normal, view, lightDir);

    //vec3 directionLight = (brdf.kD * albedo / PI + brdf.specular) * u_DirectionLight.color * max(dot(normal, lightDir), 0.0);
    
    vec3 lightColor = u_DirectionLight.color * max(dot(normal, lightDir), 0.0);
    directionLight.diffuse = (brdf.kD * albedo / PI) * lightColor;
    directionLight.specular = (brdf.specular) * lightColor;

    return directionLight;
}

LightCombination calcPointLight(vec3 albedo, float metallic, float roughness, vec3 normal, vec3 view)
{
    LightCombination result;
    result.diffuse = vec3(0.0);
    result.specular = vec3(0.0);
    
    if(u_LightConfig.pointLightNum <= 0)
        return result;

    int num = min(u_LightConfig.pointLightNum, MAX_POINT_OR_SPOT_LIGHT_NUM);
    
    float attenuation = 1.0;

    for(int i = 0; i < num; i++)
    {
        PointLight light = u_PointLights[i];
        vec3 lightDirection = light.position - v_FragPosition;
        vec3 lightDirNormal = normalize(lightDirection);
        float lightDistance = length(lightDirection);
        attenuation = 1.0 / (light.constant + light.linear * lightDistance + light.quadratic * lightDistance * lightDistance);    

        BRDF brdf = calcBRDF(albedo, metallic, roughness, normal, view, lightDirNormal);
        vec3 lightColor = light.color * max(dot(normal, lightDirNormal), 0.0) * attenuation;
        //result += (brdf.kD * albedo / PI + brdf.specular) * light.color * max(dot(normal, lightDirNormal), 0.0) * attenuation;
        result.diffuse += (brdf.kD * albedo / PI) * lightColor;
        result.specular += (brdf.specular) * lightColor;
    }

    return result;
}

LightCombination calcSpotLight(vec3 albedo, float metallic, float roughness, vec3 normal, vec3 view)
{
    LightCombination result;
    result.diffuse = vec3(0.0);
    result.specular = vec3(0.0);

    if(u_LightConfig.spotLightNum <= 0)
        return result;

    float spotFactor = 1.0;
    float attenuation = 1.0;
    int num = min(u_LightConfig.spotLightNum, MAX_POINT_OR_SPOT_LIGHT_NUM);

    vec3 lookAtLight = vec3(0.0);

    for(int i = 0; i < num; i++)
    {
        SpotLight light = u_SpotLights[i];
        lookAtLight = normalize(light.position - v_FragPosition);

        float lightDistance = length(light.position - v_FragPosition);
        float temp = dot(normalize(light.direction), -lookAtLight);

        spotFactor = clamp((temp - cos(light.outterAngle)) / (cos(light.innerAngle) - cos(light.outterAngle)), 0.0, 1.0);
        attenuation = 1.0 / (light.constant + light.linear * lightDistance + light.quadratic * lightDistance * lightDistance);

        BRDF brdf = calcBRDF(albedo, metallic, roughness, normal, view, lookAtLight);
        vec3 lightColor = light.color * max(dot(normal, lookAtLight), 0.0) * spotFactor * attenuation;
        //result += (brdf.kD * albedo / PI + brdf.specular) * light.color * max(dot(normal, lookAtLight), 0.0) * spotFactor * attenuation;
        result.diffuse += (brdf.kD * albedo / PI) * lightColor;
        result.specular += (brdf.specular) * lightColor;
    }

    return result;
}

void main() {
    vec4 albedoTex = texture(u_Material.albedo, v_TexCoord);
    vec3 albedo = albedoTex.rgb;
    float alpha = albedoTex.a;

    float metallic = (u_UseMRA ? texture(u_Material.metallic, v_TexCoord).b : texture(u_Material.metallic, v_TexCoord).r) * u_Material.metallicFactor;
    float roughness = (u_UseMRA ? texture(u_Material.roughness, v_TexCoord).g: texture(u_Material.roughness, v_TexCoord).r) * u_Material.roughnessFactor;
    float ao = texture(u_Material.AO, v_TexCoord).r;
    float transmission = texture(u_Material.transmission, v_TexCoord).r * u_Material.transmissionFactor;
    vec3 emissive = texture(u_Material.emissive, v_TexCoord).rgb;
    
    LightCombination directionLight;
    LightCombination pointLight;
    LightCombination spotLight;

    directionLight.diffuse = vec3(0.0);
    directionLight.specular = vec3(0.0);

    pointLight.diffuse = vec3(0.0);
    pointLight.specular = vec3(0.0);

    spotLight.diffuse = vec3(0.0);
    spotLight.specular = vec3(0.0);

    // gamma correction
    albedo = pow(albedo, vec3(2.2));
    emissive = pow(emissive, vec3(2.2));

    vec3 specularLight = vec3(0.0);
    vec3 diffuseLight = vec3(0.0);

    vec3 sampleNormal = texture(u_Material.normal, v_TexCoord).rgb;
    
    mat3 TBN = correctionTBN(v_T.xyz, v_N, v_T.w);

    sampleNormal = sampleNormal * 2 - 1;
    sampleNormal = TBN * sampleNormal;
    sampleNormal = normalize(sampleNormal);
    lookAtCamera_Normalized = normalize(v_LookAtCamera);

    // 折射方向
    vec2 screenTexCoord = gl_FragCoord.xy / textureSize(u_Material.background, 0);
    vec3 viewDirection = -lookAtCamera_Normalized;
    vec3 refractDirection = refract(viewDirection, sampleNormal, 1.0 / u_Material.ior);
    // 使用一个较小的强度系数（如 0.05），避免折射过于剧烈
    //vec2 refractionOffset = refractDirection.xy * 0.05 * (1.0 - roughness);

    float tirMask = step(0.00001, dot(refractDirection, refractDirection));
    vec2 refractionOffset = refractDirection.xy * 0.05 * (1.0 - roughness) * tirMask;
    vec2 refractedCoord = screenTexCoord + refractionOffset;
    
    // 采样背景颜色 (使用归一化坐标 + 折射偏移)
    vec2 finalCoord = clamp(refractedCoord, vec2(0.0), vec2(1.0));
    vec3 samplerBackground = textureLod(u_Material.background, finalCoord, 0.0).rgb;

    // transmissionColor
    float transmissionFraction = transmission * tirMask;
    vec3 finalTransmission = samplerBackground * albedo * transmissionFraction;

    vec3 ambient = calcIBLLight(sampleNormal, -lookAtCamera_Normalized, albedo, roughness, metallic, mix(vec3(0.04), albedo, metallic)) * ao;

    directionLight = calcDirectionLight(albedo, metallic, roughness, sampleNormal, -lookAtCamera_Normalized);
    pointLight = calcPointLight(albedo, metallic, roughness, sampleNormal, -lookAtCamera_Normalized);
    spotLight = calcSpotLight(albedo, metallic, roughness, sampleNormal, -lookAtCamera_Normalized);

    diffuseLight = directionLight.diffuse + pointLight.diffuse + spotLight.diffuse;
    specularLight = directionLight.specular + pointLight.specular + spotLight.specular;

    color = vec4(ambient + specularLight + (1 - transmission) * diffuseLight + transmission * finalTransmission + emissive * 10.0, alpha); 
}

)glsl";