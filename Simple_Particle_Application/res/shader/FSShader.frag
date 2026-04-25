#version 330 core

uniform sampler2D u_Texture[4]; // 纹理组
uniform vec3 u_LightColor;      // 光照颜色

in float v_TexSlot;
in vec2 v_TexCoord;
in vec4 v_Color;
in vec3 v_Normal;
in vec3 v_LookAtLight;
in vec3 v_LookAtCamera;

out vec4 color;


void main() {
    int slot = int(v_TexSlot + 0.5);
    float ambient = 0.3;
    float diffuse = 0.0;
    float specular = 0.0;
    float reflectRate = 0.5;
    vec4 textureColor = texture(u_Texture[slot], v_TexCoord);

    /* Normalize again!!! Must!!! */
    vec3 norm = normalize(v_Normal);
    vec3 lightDir = normalize(v_LookAtLight);
    vec3 viewDir = normalize(v_LookAtCamera);

    diffuse = max(dot(lightDir, norm), 0.0);
    
    vec3 reflectVec = reflect(-lightDir, norm);
    specular = pow(max(dot(viewDir, reflectVec), 0.0), 128) * reflectRate;

    vec3 specularHighlight = specular * u_LightColor;
    vec3 rgb = (ambient + diffuse) * u_LightColor * textureColor.rgb + specularHighlight;
    color = vec4(rgb, textureColor.a);

    //color = v_Color;

    /* 注意，这里是将纹理上，对应坐标的颜色RGB完完整整取出来，不会管Alpha通道的值的，就算Alpha是0，也会被完整取出来，写入到frame buffer中 */
    /* 所以要么在OpenGL开启Blend，要么在这里，自行过滤： */
    //if(color.a < 0.1)
    //{
        //discard;
    //}
};