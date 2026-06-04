#pragma once

// GBuffer fragment shader.
// language=GLSL
static const char fsShaderCodeDR[] = R"glsl(
#version 460 core
layout (location = 0) out vec3 gPosition;               // 世界坐标
layout (location = 1) out vec3 gLightSpacePosition;     // NDC坐标
layout (location = 2) out vec3 gN;
layout (location = 3) out vec4 gAlbedoSpec;

struct Material {
    sampler2D ambient;
    sampler2D diffuse;  // 材质漫反射贴图
    sampler2D specular; // 材质高光反射贴图
    sampler2D normal;   // 材质法线贴图
    int shininess;      // 材质镜面反射率（控制高光光斑的大小）
};

uniform Material u_Material;

in vec2 v_TexCoord;
in vec3 v_FragPosition;
in vec4 v_LightSpacePosition;
in vec3 v_T;
in vec3 v_B;
in vec3 v_N;

mat3 correctionTBN(vec3 v_T, vec3 v_B, vec3 v_N)
{
    vec3 T = normalize(v_T);
    vec3 B = normalize(v_B);
    vec3 N = normalize(v_N);

    T = normalize(T - dot(T,N)*N);

    float handedness = sign(dot(cross(N, T), B));
    B = cross(N, T) * handedness;

    mat3 TBN = mat3(T,B,N);

    return TBN;
}

void main() {
    gPosition = v_FragPosition;
    gLightSpacePosition = v_LightSpacePosition.xyz / v_LightSpacePosition.w;

    mat3 TBN = correctionTBN(v_T, v_B, v_N);
    vec3 normalVec = texture(u_Material.normal, v_TexCoord).rgb;
    
    normalVec = normalVec * 2 - 1;
    normalVec = TBN * normalVec;
    normalVec = normalize(normalVec);
    
    gN = normalVec;
    gAlbedoSpec = vec4(texture(u_Material.diffuse, v_TexCoord).rgb, texture(u_Material.specular, v_TexCoord).a);

    // check position
    // color = vec4(gPosition, 1.0);

    // check normal
    //color = vec4(normalVec * 0.5 + 0.5, 1.0);

    // check light space position
    //color = vec4(normalize(gLightSpacePosition) * 0.5 + 0.5, 1.0);

    // check albedo
    //color = vec4(gAlbedoSpec.rgb, 1.0);

    // check specular
    //color = vec4(gAlbedoSpec.a, gAlbedoSpec.a, gAlbedoSpec.a, 1.0);
}
)glsl";

// Final lighting shader
// language=GLSL
static const char fsShaderCodeLight[] = R"glsl(
#version 460 core

const int MAX_POINT_OR_SPOT_LIGHT_NUM = 4;

struct LightColor {
    vec3 ambient;   // 环境光的颜色
    vec3 diffuse;   // 光源漫反射光的颜色
    vec3 specular;  // 光源高光的颜色
};

/* 点光源 */
struct PointLight {
    vec3 position;

    float constant;  // 常数衰减系数
    float linear;    // 线性衰减系数
    float quadratic; // 二次衰减系数   

    LightColor color;
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

    LightColor color;
};

/* 方向光 */
struct DirectionLight {
    vec3 direction; // 光源方向

    LightColor color;
};

/* 光源配置 */
struct LightConfig {
    bool enableDirectionLight;  // 方向光的开关
    int spotLightNum;           // 点光源个数，做多支持MAX_POINT_OR_SPOT_NUM这么多个，这个数量没有什么依据，我自己说的
    int pointLightNum;          // 点光源个数，做多支持MAX_POINT_OR_SPOT_NUM这么多个，这个数量没有什么依据，我自己说的
};

uniform LightConfig u_LightConfig;
uniform DirectionLight u_DirectionLight;
uniform PointLight u_PointLights[MAX_POINT_OR_SPOT_LIGHT_NUM];
uniform SpotLight u_SpotLights[MAX_POINT_OR_SPOT_LIGHT_NUM];
uniform vec3 u_CameraPosition;
uniform float u_FarPlane;
uniform sampler2D u_PositionMap;
uniform sampler2D u_DLightSpacePositionMap;
uniform sampler2D u_NormalMap;
uniform sampler2D u_AlbedoSpecMap;
uniform sampler2D u_ShadowDepthMap;
uniform samplerCube u_PointLightsDepthMap[MAX_POINT_OR_SPOT_LIGHT_NUM];

vec3 lookAtCamera_Normalized;
vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);
float reflectDamping = 1.0; // 反射衰减系数，镜面反射光的强度衰减

in vec2 v_TexCoord;

out vec4 color;

vec3 calcDirectionLight(vec3 sampleColorAmbient, vec3 sampleColorDiffuse, vec3 sampleColorSpecular)
{
    if(!u_LightConfig.enableDirectionLight)
        return vec3(0.0);

    vec3 result = vec3(0.0);

    ivec2 depthMapSize = textureSize(u_ShadowDepthMap, 0);
    vec2 texelSize = vec2(1.0 / depthMapSize.x, 1.0 / depthMapSize.y);

    float diffuse = 0.0;
    float shadowDebuff = 0.0;

    // 1. 将裁剪空间坐标转换到 [0, 1] 的 NDC 空间
    vec3 shadowMapPositionNDC = texture(u_DLightSpacePositionMap, v_TexCoord).rgb * 0.5 + 0.5;
    // 2. 正确使用 NDC 空间的 xy 作为 UV 坐标去采样阴影图
    vec2 shadowMapTexCoord = shadowMapPositionNDC.xy;

    // 3. 跟深度一个偏移值，消除摩尔纹
    vec3 lightDir = normalize(-u_DirectionLight.direction);
    vec3 normal_Normalized = texture(u_NormalMap, v_TexCoord).rgb;
    float bias = max(0.005 * (1.0 - dot(normal_Normalized, lightDir)), 0.0005);

    for(int x = -1; x <= 1; x++)
        for(int y = -1; y <= 1; y++)
        {
    
            float shadowMapDepth = texture(u_ShadowDepthMap, shadowMapTexCoord + vec2(x, y) * texelSize).r;
            shadowDebuff += (shadowMapPositionNDC.z - bias) > shadowMapDepth ? 0.0 : 1.0;
        }

    shadowDebuff /= 9.0;
    shadowDebuff = shadowMapPositionNDC.z > 1.0 ? 1.0 : shadowDebuff;
        
    vec3 ambientLight = u_DirectionLight.color.ambient * sampleColorAmbient;
    vec3 diffuseLight = u_DirectionLight.color.diffuse * max(dot(normalize(-u_DirectionLight.direction),normal_Normalized), 0.0) * sampleColorDiffuse;
    vec3 specularLight = u_DirectionLight.color.specular * pow(max(dot(lookAtCamera_Normalized, reflect(normalize(u_DirectionLight.direction), normal_Normalized)), 0.0), 8) * sampleColorSpecular * reflectDamping;

    result = ambientLight + diffuseLight * shadowDebuff + specularLight * shadowDebuff;
    return result;
}

vec3 calcPointLights(vec3 sampleColorAmbient, vec3 sampleColorDiffuse, vec3 sampleColorSpecular)
{
    if(u_LightConfig.pointLightNum <= 0)
        return vec3(0.0);

    vec3 result = vec3(0.0);

    float diffuse = 0.0;
    float specular = 0.0;
    float attenuation = 1.0; // 聚光源随着距离衰减的系数
    
    vec3 diffuseLight = vec3(0.0);
    vec3 specularLight = vec3(0.0);
    vec3 ambientLight = vec3(0.0);
    vec3 lookAtLight = vec3(0.0);

    vec3 fragPosition = texture(u_PositionMap, v_TexCoord).rgb;
    vec3 lookAtCamera = u_CameraPosition - fragPosition;
    vec3 normal_Normalized = texture(u_NormalMap, v_TexCoord).rgb;

    int num = min(u_LightConfig.pointLightNum, MAX_POINT_OR_SPOT_LIGHT_NUM);
    int samples = 20;
    float viewDistance = length(lookAtCamera);
    float diskRadius = 0.05;
    
    for (int i = 0; i < num; ++i)
    {
        PointLight light = u_PointLights[i];
        lookAtLight = light.position - fragPosition;

        float lightDistance = length(lookAtLight);
        attenuation = 1.0 / (light.constant + light.linear * lightDistance + light.quadratic * lightDistance * lightDistance);
        
        ambientLight = light.color.ambient * sampleColorAmbient;

        diffuse = max(dot(normal_Normalized, normalize(lookAtLight)), 0.0);
        diffuseLight = diffuse * light.color.diffuse * sampleColorDiffuse;

        float bias = 0.05;
        float shadowDebuff = 0.0;
        
        for(int j = 0; j < samples; ++j)
        {
            float closestDepth = texture(u_PointLightsDepthMap[i], normalize(-lookAtLight) + sampleOffsetDirections[j] * diskRadius).r;
            closestDepth *= u_FarPlane;
            
            shadowDebuff += ((lightDistance - bias < closestDepth) ? 1.0 : 0.0);
        }
        
       shadowDebuff /= float(samples);

        vec3 reflectVec = reflect(-normalize(lookAtLight), normal_Normalized);
        vec3 halfWayVec = normalize(lookAtCamera_Normalized + normalize(lookAtLight));
        specular = pow(max(dot(normal_Normalized, halfWayVec), 0.0), 8);
        specularLight = specular * light.color.specular * sampleColorSpecular * reflectDamping;
        result += ((ambientLight + diffuseLight * shadowDebuff + specularLight * shadowDebuff) * attenuation);
    }
    return result;
}

vec3 calcSpotLight(vec3 sampleColorAmbient, vec3 sampleColorDiffuse, vec3 sampleColorSpecular)
{
     if(u_LightConfig.spotLightNum <= 0)
        return vec3(0.0);

     vec3 result = vec3(0.0);

     float diffuse = 0.0;
     float specular = 0.0;
     float spotFactor = 1.0;
     float attenuation = 1.0; // 聚光源随着距离衰减的系数

     vec3 diffuseLight = vec3(0.0);
     vec3 specularLigt = vec3(0.0);
     vec3 ambientLight = vec3(0.0);
     vec3 lookAtLight = vec3(0.0);

     vec3 fragPosition = texture(u_PositionMap, v_TexCoord).rgb;
     vec3 normal_Normalized = texture(u_NormalMap, v_TexCoord).rgb;

     int num = min(u_LightConfig.spotLightNum, MAX_POINT_OR_SPOT_LIGHT_NUM);
     for(int i = 0; i < num; i++)
     {
        SpotLight light = u_SpotLights[i];

        lookAtLight = light.position - fragPosition;
        float lightDistance = length(light.position - fragPosition);
        float temp = dot(normalize(light.direction), normalize(-lookAtLight));
        
        spotFactor = clamp((temp - cos(light.outterAngle)) / (cos(light.innerAngle) - cos(light.outterAngle)), 0.0, 1.0);
        attenuation = 1.0 / (light.constant + light.linear * lightDistance + light.quadratic * lightDistance * lightDistance);
        ambientLight = light.color.ambient * sampleColorAmbient;
        
        diffuse = max(dot(normal_Normalized, normalize(lookAtLight)), 0.0);
        diffuseLight = diffuse * light.color.diffuse * sampleColorDiffuse;

        vec3 reflectVec = reflect(-normalize(lookAtLight), normal_Normalized);
        specular = pow(max(dot(lookAtCamera_Normalized, reflectVec), 0.0), 8);
        specularLigt = specular * light.color.specular * sampleColorSpecular * reflectDamping;
        result += ((ambientLight + diffuseLight + specularLigt) * spotFactor * attenuation + (1 - spotFactor) * ambientLight * attenuation);
     }

     return result;
}

void main() {
    vec4 albedoSpec = texture(u_AlbedoSpecMap, v_TexCoord);
    vec3 sampleColorAmbient = albedoSpec.rgb;
    vec3 sampleColorDiffuse = albedoSpec.rgb;
    vec3 sampleColorSpecular = vec3(albedoSpec.a, albedoSpec.a, albedoSpec.a);
    
    sampleColorAmbient = pow(sampleColorAmbient, vec3(2.2));
    sampleColorDiffuse = pow(sampleColorDiffuse, vec3(2.2));
    sampleColorSpecular = pow(sampleColorSpecular, vec3(2.2));

    vec3 light = vec3(0.0, 0.0, 0.0);

    lookAtCamera_Normalized = normalize(u_CameraPosition - texture(u_PositionMap, v_TexCoord).rgb);
    light = calcDirectionLight(sampleColorDiffuse, sampleColorDiffuse, sampleColorSpecular) + calcPointLights(sampleColorDiffuse, sampleColorDiffuse, sampleColorSpecular) + calcSpotLight(sampleColorDiffuse, sampleColorDiffuse, sampleColorSpecular);
    color = vec4(light, 1.0);
}
)glsl";