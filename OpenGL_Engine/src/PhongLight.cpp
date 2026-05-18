#include <sstream>
#include "PhongLight.h"

#include <glm/gtc/constants.hpp>

// language=GLSL
static const char vsShaderCode[] = R"glsl(
#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texCoord;
layout(location = 3) in vec3 normal;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;
uniform vec3 u_CameraPosition;	// 相机世界坐标
uniform mat3 u_NormalMat;       // 法线变换矩阵(模型变换矩阵左上角3x3的逆矩阵的转置矩阵)

out vec2 v_TexCoord;
out vec3 v_Normal;
out vec3 v_LookAtLight;
out vec3 v_LookAtCamera;
out vec3 v_FragPosition;

void main() {
	gl_Position = u_Projection * u_View * u_Model * position;
	vec4 worldPosition = u_Model * position;
	v_TexCoord = texCoord;
	v_Normal = normalize(u_NormalMat * normal);
	v_LookAtCamera = normalize(u_CameraPosition.xyz - worldPosition.xyz);
	v_FragPosition = vec3(worldPosition);
}
)glsl";

// language=GLSL
static const char fsShaderCode[] = R"glsl(
#version 330 core

const int MAX_POINT_OR_SPOT_LIGHT_NUM = 4;

struct Material {
    sampler2D diffuse;  // 材质漫反射贴图
    sampler2D specular; // 材质高光反射贴图
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

in vec2 v_TexCoord;
in vec3 v_Normal;
in vec3 v_LookAtCamera;
in vec3 v_FragPosition;

out vec4 color;

vec3 normal_Normalized;
vec3 lookAtCamera_Normalized;
float reflectDamping = 1.0; // 反射衰减系数，镜面反射光的强度衰减

vec3 calcDirectionLight(vec3 sampleColorDiffuse, vec3 sampleColorSpecular)
{
    if(!u_LightConfig.enableDirectionLight)
        return vec3(0.0);

    vec3 result = vec3(0.0);

    float diffuse = 0.0;
    float specualr = 0.0;
    
        
    vec3 specularLigt = vec3(0.0);
    vec3 ambientLight = u_DirectionLight.color.ambient * sampleColorDiffuse;
    vec3 diffuseLight = u_DirectionLight.color.diffuse * max(dot(normalize(-u_DirectionLight.direction),normal_Normalized), 0.0) * sampleColorDiffuse;
    vec3 specularLight = u_DirectionLight.color.specular * pow(max(dot(lookAtCamera_Normalized, reflect(normalize(u_DirectionLight.direction), normal_Normalized)), 0.0), u_Material.shininess) * sampleColorSpecular * reflectDamping;

    result = ambientLight + diffuseLight + specularLight;

    return result;
}

vec3 calcPointLights(vec3 sampleColorDiffuse, vec3 sampleColorSpecular)
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
    for (int i = 0; i < num; ++i)
    {
        PointLight light = u_PointLights[i];
        lookAtLight = light.position - v_FragPosition;

        float lightDistance = length(light.position - v_FragPosition);
        attenuation = 1.0 / (light.constant + light.linear * lightDistance + light.quadratic * lightDistance * lightDistance);
        
        ambientLight = light.color.ambient * sampleColorDiffuse;

        diffuse = max(dot(normal_Normalized, normalize(lookAtLight)), 0.0);
        diffuseLight = diffuse * light.color.diffuse * sampleColorDiffuse;

        vec3 reflectVec = reflect(-normalize(lookAtLight), normal_Normalized);
        vec3 halfWayVec = normalize(lookAtCamera_Normalized + lookAtLight);
        specular = pow(max(dot(normal_Normalized, halfWayVec), 0.0), u_Material.shininess);
        specularLight = specular * light.color.specular * sampleColorSpecular * reflectDamping;
        result += ((ambientLight + diffuseLight + specularLight) * attenuation);
    }
    return result;
}

vec3 calcSpotLight(vec3 sampleColorDiffuse, vec3 sampleColorSpecular)
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
        ambientLight = light.color.ambient * sampleColorDiffuse;
        
        diffuse = max(dot(normal_Normalized, normalize(lookAtLight)), 0.0);
        diffuseLight = diffuse * light.color.diffuse * sampleColorDiffuse;

        vec3 reflectVec = reflect(-normalize(lookAtLight), normal_Normalized);
        specular = pow(max(dot(lookAtCamera_Normalized, reflectVec), 0.0), u_Material.shininess);
        specularLigt = specular * light.color.specular * sampleColorSpecular * reflectDamping;
        result += ((ambientLight + diffuseLight + specularLigt) * spotFactor * attenuation + (1 - spotFactor) * ambientLight * attenuation);
     }

     return result;
}

void main() {
    vec3 sampleColorDiffuse = texture(u_Material.diffuse, v_TexCoord).rgb;
    vec3 sampleColorSpecular = texture(u_Material.specular, v_TexCoord).rgb;
    //vec3 sampleColorSpecular = vec3(0.5, 0.5, 0.5);
    vec3 light = vec3(0.0, 0.0, 0.0);

    // gamma correction
    sampleColorDiffuse = pow(sampleColorDiffuse, vec3(2.2));

    normal_Normalized = normalize(v_Normal);
    lookAtCamera_Normalized = normalize(v_LookAtCamera);

    light = calcDirectionLight(sampleColorDiffuse, sampleColorSpecular) + calcPointLights(sampleColorDiffuse, sampleColorSpecular) + calcSpotLight(sampleColorDiffuse, sampleColorSpecular);

    color = vec4(light, texture(u_Material.diffuse, v_TexCoord).a);

    // gamma correction
    color = vec4(pow(color.rgb, vec3(1.0/2.2)), color.a);

    /* 注意，这里是将纹理上，对应坐标的颜色RGB完完整整取出来，不会管Alpha通道的值的，就算Alpha是0，也会被完整取出来，写入到frame buffer中 */
    /* 所以要么在OpenGL开启Blend，要么在这里，自行过滤： */
    //if(color.a < 0.1)
    //{
        //discard;
    //}
}
)glsl";

static const int MAX_NON_DIRECTIONAL_LIGHTS = 4;

Engine::PhongLight::PhongLight() : m_Shader(vsShaderCode, fsShaderCode)
{
    m_Shader.Use();
    
    /* 默认只开启方向光源 */
    m_Shader.SetUniform1i("u_LightConfig.enableDirectionLight", 1);	// 开启方向光源
    m_Shader.SetUniform1i("u_LightConfig.pointLightNum", 0);			// 不使用点光源
    m_Shader.SetUniform1i("u_LightConfig.spotLightNum", 0);			// 不使用聚光源
    
    /* 方向光配置 */
    m_Shader.SetUniform3f("u_DirectionLight.direction", 1.0f, 1.0f, 1.0f);
    m_Shader.SetUniform3f("u_DirectionLight.color.ambient", 0.5f, 0.5f, 0.5f);
    m_Shader.SetUniform3f("u_DirectionLight.color.diffuse", 1.0f, 1.0f, 1.0f);
    m_Shader.SetUniform3f("u_DirectionLight.color.specular", 0.7f, 0.7f, 0.7f);
    
    m_Shader.UnUse();
}

void Engine::PhongLight::TurnOn() const
{   
    if (!m_Shader.CheckShaderValidity())
        return;
    
    m_Shader.Use();
}

void Engine::PhongLight::TurnOff() const
{
    if (!m_Shader.CheckShaderValidity())
        return;
    
    m_Shader.UnUse();
}

/* Directional Light */

void Engine::PhongLight::EnableDirectionLight()
{
    if (!m_Shader.CheckShaderValidity())
        return;
    
    m_Shader.Use();
    m_Shader.SetUniform1i("u_LightConfig.enableDirectionLight", 1);
    m_Shader.UnUse();
}

void Engine::PhongLight::DisableDirectionLight()
{
    if (!m_Shader.CheckShaderValidity())
        return;
    
    m_Shader.Use();
    m_Shader.SetUniform1i("u_LightConfig.enableDirectionLight", 0);
    m_Shader.UnUse();
}

void Engine::PhongLight::ConfigDirectionLight(const vec3& direction, const vec3& ambient, const vec3& diffuse, const vec3& specular)
{
    if (!m_Shader.CheckShaderValidity())
        return;
    
    m_Shader.Use();
    m_Shader.SetUniform3f("u_DirectionLight.direction", direction.x, direction.y, direction.z);
    m_Shader.SetUniform3f("u_DirectionLight.color.ambient", ambient.x, ambient.y, ambient.z);
    m_Shader.SetUniform3f("u_DirectionLight.color.diffuse", diffuse.x, diffuse.y, diffuse.z);
    m_Shader.SetUniform3f("u_DirectionLight.color.specular", specular.x, specular.y, specular.z);
    m_Shader.UnUse();
}

/* Point Light */

bool Engine::PhongLight::AddPointLight(const vec3& position, const vec3& ambient, const vec3& diffuse, const vec3& specular, float constant, float linear, float quadratic)
{
    if (!m_Shader.CheckShaderValidity() || m_PointLightIndex >= MAX_NON_DIRECTIONAL_LIGHTS)
        return false;
    
    std::string uniformName = "u_PointLights[" + std::to_string(m_PointLightIndex) + "].";
    
    m_Shader.Use();
    
    m_Shader.SetUniform3f(uniformName + "position", position.x, position.y, position.z);
    m_Shader.SetUniform1f(uniformName + "constant", constant);
    m_Shader.SetUniform1f(uniformName + "linear", linear);
    m_Shader.SetUniform1f(uniformName + "quadratic", quadratic);
    m_Shader.SetUniform3f(uniformName + "color.ambient", ambient.x, ambient.y, ambient.z);
    m_Shader.SetUniform3f(uniformName + "color.diffuse", diffuse.x, diffuse.y, diffuse.z);
    m_Shader.SetUniform3f(uniformName + "color.specular", specular.x, specular.y, specular.z);
    m_Shader.SetUniform1i("u_LightConfig.pointLightNum", ++m_PointLightIndex);
    
    m_Shader.UnUse();
    
    return true;
}

bool Engine::PhongLight::ConfigPointLightPosition(int index, const vec3& position)
{
    if (!m_Shader.CheckShaderValidity() || index >= MAX_NON_DIRECTIONAL_LIGHTS)
        return false;
    
    std::string uniformName = "u_PointLights[" + std::to_string(index) + "].";
    
    m_Shader.Use();
    m_Shader.SetUniform3f(uniformName + "position", position.x, position.y, position.z);
    m_Shader.UnUse();
    
    return true;
}

bool Engine::PhongLight::ConfigPointLightColor(int index, const vec3& ambient, const vec3& diffuse, const vec3& specular)
{
    if (!m_Shader.CheckShaderValidity() || index >= MAX_NON_DIRECTIONAL_LIGHTS)
        return false;
    
    std::string uniformName = "u_PointLights[" + std::to_string(index) + "].";
    
    m_Shader.Use();
    m_Shader.SetUniform3f(uniformName + "color.ambient", ambient.x, ambient.y, ambient.z);
    m_Shader.SetUniform3f(uniformName + "color.diffuse", diffuse.x, diffuse.y, diffuse.z);
    m_Shader.SetUniform3f(uniformName + "color.specular", specular.x, diffuse.y, diffuse.z);
    m_Shader.UnUse();
    
    return true;
}

bool Engine::PhongLight::ConfigPointLightAttenuation(int index, float constant, float linear, float quadratic)
{
    if (!m_Shader.CheckShaderValidity() || index >= MAX_NON_DIRECTIONAL_LIGHTS)
        return false;
    
    std::string uniformName = "u_PointLights[" + std::to_string(index) + "].";
    
    m_Shader.Use();
    m_Shader.SetUniform1f(uniformName + "constant", constant);
    m_Shader.SetUniform1f(uniformName + "linear", linear);
    m_Shader.SetUniform1f(uniformName + "quadratic", quadratic);
    m_Shader.UnUse();
    
    return true;
}

bool Engine::PhongLight::AddSpotLight(const vec3& position, const vec3& direction, const vec3& ambient, const vec3& diffuse, const vec3& specular, float innerAngle, float outterAngle, float constant, float linear, float quadratic)
{
    if (!m_Shader.CheckShaderValidity() || m_SpotLightIndex >= MAX_NON_DIRECTIONAL_LIGHTS)
        return false;
    
    std::string uniformName = "u_SpotLights[" + std::to_string(m_SpotLightIndex) + "].";
    
    m_Shader.Use();
    
    m_Shader.SetUniform3f(uniformName + "position", position.x, position.y, position.z);
    m_Shader.SetUniform3f(uniformName + "direction", direction.x, direction.y, direction.z);
    m_Shader.SetUniform1f(uniformName + "innerAngle", glm::radians(innerAngle));
    m_Shader.SetUniform1f(uniformName + "outterAngle", glm::radians(outterAngle));
    m_Shader.SetUniform1f(uniformName + "constant", constant);
    m_Shader.SetUniform1f(uniformName + "linear", linear);
    m_Shader.SetUniform1f(uniformName + "quadratic", quadratic);
    m_Shader.SetUniform3f(uniformName + "color.ambient", ambient.x, ambient.y, ambient.z);
    m_Shader.SetUniform3f(uniformName + "color.diffuse", diffuse.x, diffuse.y, diffuse.z);
    m_Shader.SetUniform3f(uniformName + "color.specular", specular.x, specular.y, specular.z);
    m_Shader.SetUniform1i("u_LightConfig.spotLightNum", ++m_SpotLightIndex);
    
    m_Shader.UnUse();
    
    return true;
}

bool Engine::PhongLight::ConfigSpotLightPosition(int index, const vec3& position)
{
    if (!m_Shader.CheckShaderValidity() || index >= MAX_NON_DIRECTIONAL_LIGHTS)
        return false;
    
    std::string uniformName = "u_SpotLights[" + std::to_string(index) + "].";
    
    m_Shader.Use();
    m_Shader.SetUniform3f(uniformName + "position", position.x, position.y, position.z);
    m_Shader.UnUse();
    
    return true;
}

bool Engine::PhongLight::ConfigSpotLightDirection(int index, const vec3& direction)
{
    if (!m_Shader.CheckShaderValidity() || index >= MAX_NON_DIRECTIONAL_LIGHTS)
        return false;
    
    std::string uniformName = "u_SpotLights[" + std::to_string(index) + "].";
    
    m_Shader.Use();
    m_Shader.SetUniform3f(uniformName + "direction", direction.x, direction.y, direction.z);
    m_Shader.UnUse();
    
    return true;
}


bool Engine::PhongLight::ConfigSpotLightColor(int index, const vec3& ambient, const vec3& diffuse, const vec3& specular)
{
    if (!m_Shader.CheckShaderValidity() || index >= MAX_NON_DIRECTIONAL_LIGHTS)
        return false;
    
    std::string uniformName = "u_SpotLights[" + std::to_string(index) + "].";
    
    m_Shader.Use();
    m_Shader.SetUniform3f(uniformName + "color.ambient", ambient.x, ambient.y, ambient.z);
    m_Shader.SetUniform3f(uniformName + "color.diffuse", diffuse.x, diffuse.y, diffuse.z);
    m_Shader.SetUniform3f(uniformName + "color.specular", specular.x, specular.y, specular.z);
    m_Shader.UnUse();
    
    return true;
}

bool Engine::PhongLight::ConfigSpotLightAttenuation(int index, float constant, float linear, float quadratic)
{
    if (!m_Shader.CheckShaderValidity() || index >= MAX_NON_DIRECTIONAL_LIGHTS)
        return false;
    
    std::string uniformName = "u_SpotLights[" + std::to_string(index) + "].";
    
    m_Shader.Use();
    m_Shader.SetUniform1f(uniformName + "constant", constant);
    m_Shader.SetUniform1f(uniformName + "linear", linear);
    m_Shader.SetUniform1f(uniformName + "quadratic", quadratic);
    m_Shader.UnUse();
    
    return true;
}

bool Engine::PhongLight::ConfitSpotLightScale(int index, float innerAngle, float outterAngle)
{
    if (!m_Shader.CheckShaderValidity() || index >= MAX_NON_DIRECTIONAL_LIGHTS)
        return false;
    
    std::string uniformName = "u_SpotLights[" + std::to_string(index) + "].";
    
    m_Shader.Use();
    m_Shader.SetUniform1f(uniformName + "innerAngle", innerAngle);
    m_Shader.SetUniform1f(uniformName + "outterAngle", outterAngle);
    m_Shader.UnUse();
    
    return true;
}

void Engine::PhongLight::ConfigMVPMatrix(const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
    if (!m_Shader.CheckShaderValidity())
        return;
    
    m_Shader.Use();
    m_Shader.SetUniformMatrix4f("u_Model", modelMatrix);
    m_Shader.SetUniformMatrix4f("u_View", viewMatrix);
    m_Shader.SetUniformMatrix4f("u_Projection", projectionMatrix);
    m_Shader.UnUse();
}

void Engine::PhongLight::ConfigNormalMatrix(const glm::mat3& normalMatrix)
{
    if (!m_Shader.CheckShaderValidity())
        return;
    
    m_Shader.Use();
    m_Shader.SetUniformMatrix3f("u_NormalMat", normalMatrix);
    m_Shader.UnUse();
}

void Engine::PhongLight::ConfigCameraWorldPosition(const glm::vec3& position)
{
    if (!m_Shader.CheckShaderValidity())
        return;
    
    m_Shader.Use();
    m_Shader.SetUniform3f("u_CameraPosition", position.x, position.y, position.z);
    m_Shader.UnUse();
}
