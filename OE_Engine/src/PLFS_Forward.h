#pragma once

// language=GLSL
static const char fsShaderCodeForward[] = R"glsl(
#version 460 core

const int MAX_POINT_OR_SPOT_LIGHT_NUM = 4;

struct Material {
    sampler2D ambient;
    sampler2D diffuse;  // 材质漫反射贴图
    sampler2D specular; // 材质高光反射贴图
    sampler2D normal;   // 材质法线贴图
    int shininess;      // 材质镜面反射率（控制高光光斑的大小）
};

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
uniform Material u_Material;
uniform sampler2D u_ShadowDepthMap;
uniform float u_FarPlane;
uniform samplerCube u_PointLightsDepthMap[MAX_POINT_OR_SPOT_LIGHT_NUM];

in vec2 v_TexCoord;
in vec3 v_Normal;
in vec3 v_LookAtCamera;
in vec3 v_FragPosition;
in vec4 v_LightSpacePosition;
in vec3 v_T;
in vec3 v_B;
in vec3 v_N;

out vec4 color;

vec3 normal_Normalized;
vec3 lookAtCamera_Normalized;
float reflectDamping = 1.0; // 反射衰减系数，镜面反射光的强度衰减
vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

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
    vec3 shadowMapPositionNDC = (v_LightSpacePosition.xyz / v_LightSpacePosition.w) * 0.5 + 0.5;
    // 2. 正确使用 NDC 空间的 xy 作为 UV 坐标去采样阴影图
    vec2 shadowMapTexCoord = shadowMapPositionNDC.xy;

    // 3. 跟深度一个偏移值，消除摩尔纹
    vec3 lightDir = normalize(-u_DirectionLight.direction);
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
    vec3 specularLight = u_DirectionLight.color.specular * pow(max(dot(lookAtCamera_Normalized, reflect(normalize(u_DirectionLight.direction), normal_Normalized)), 0.0), u_Material.shininess) * sampleColorSpecular * reflectDamping;

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

    int num = min(u_LightConfig.pointLightNum, MAX_POINT_OR_SPOT_LIGHT_NUM);
    int samples = 20;
    float viewDistance = length(v_LookAtCamera);
    float diskRadius = 0.05;
    for (int i = 0; i < num; ++i)
    {
        PointLight light = u_PointLights[i];
        lookAtLight = light.position - v_FragPosition;

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
        specular = pow(max(dot(normal_Normalized, halfWayVec), 0.0), u_Material.shininess);
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

     int num = min(u_LightConfig.spotLightNum, MAX_POINT_OR_SPOT_LIGHT_NUM);
     for(int i = 0; i < num; i++)
     {
        SpotLight light = u_SpotLights[i];

        lookAtLight = light.position - v_FragPosition;
        float lightDistance = length(light.position - v_FragPosition);
        float temp = dot(normalize(light.direction), normalize(-lookAtLight));
        
        spotFactor = clamp((temp - cos(light.outterAngle)) / (cos(light.innerAngle) - cos(light.outterAngle)), 0.0, 1.0);
        attenuation = 1.0 / (light.constant + light.linear * lightDistance + light.quadratic * lightDistance * lightDistance);
        ambientLight = light.color.ambient * sampleColorAmbient;
        
        diffuse = max(dot(normal_Normalized, normalize(lookAtLight)), 0.0);
        diffuseLight = diffuse * light.color.diffuse * sampleColorDiffuse;

        vec3 reflectVec = reflect(-normalize(lookAtLight), normal_Normalized);
        specular = pow(max(dot(lookAtCamera_Normalized, reflectVec), 0.0), u_Material.shininess);
        specularLigt = specular * light.color.specular * sampleColorSpecular * reflectDamping;
        result += ((ambientLight + diffuseLight + specularLigt) * spotFactor * attenuation + (1 - spotFactor) * ambientLight * attenuation);
     }

     return result;
}

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
    vec3 sampleColorDiffuse = texture(u_Material.diffuse, v_TexCoord).rgb;
    vec3 sampleColorSpecular = texture(u_Material.specular, v_TexCoord).rgb;
    vec3 sampleColorAmbient = texture(u_Material.ambient, v_TexCoord).rgb; // TODO: 环境反射(比如天空盒)用的，现在暂时用不上
    vec3 light = vec3(0.0, 0.0, 0.0);
    vec3 sampleNormal = texture(u_Material.normal, v_TexCoord).rgb;

    // gamma correction
    sampleColorAmbient = pow(sampleColorAmbient, vec3(2.2));
    sampleColorDiffuse = pow(sampleColorDiffuse, vec3(2.2));
    sampleColorSpecular = pow(sampleColorSpecular, vec3(2.2));
    //sampleNormal = pow(sampleNormal, vec3(2.2));

    mat3 TBN = correctionTBN(v_T, v_B, v_N);

    sampleNormal = sampleNormal * 2 - 1;
    sampleNormal = TBN * sampleNormal;

    normal_Normalized = normalize(sampleNormal);
    //normal_Normalized = normalize(v_Normal);
    lookAtCamera_Normalized = normalize(v_LookAtCamera);

    light = calcDirectionLight(sampleColorDiffuse, sampleColorDiffuse, sampleColorSpecular) + calcPointLights(sampleColorDiffuse, sampleColorDiffuse, sampleColorSpecular) + calcSpotLight(sampleColorDiffuse, sampleColorDiffuse, sampleColorSpecular);

    color = vec4(light, texture(u_Material.diffuse, v_TexCoord).a);

    // gamma correction
    //color = vec4(pow(color.rgb, vec3(1.0/2.2)), color.a);

    /* 注意，这里是将纹理上，对应坐标的颜色RGB完完整整取出来，不会管Alpha通道的值的，就算Alpha是0，也会被完整取出来，写入到frame buffer中 */
    /* 所以要么在OpenGL开启Blend，要么在这里，自行过滤： */
    //if(color.a < 0.1)
    //{
        //discard;
    //}
}
)glsl";