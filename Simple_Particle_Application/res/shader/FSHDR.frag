#version 460 core

/* Attention pls! */
/* Gamma correction won't be done in this shader. */
/* Make sure it samples from texture that gamma correction has been done. */

uniform sampler2D u_Diffuse;
uniform float u_Exposure;
uniform bool u_HDR;

in  vec2 v_TexCoord;
out vec4 color;

void main() {
    float gamma = 2.2;
    if (u_HDR)
    {
        vec3 hdrColor = texture(u_Diffuse, v_TexCoord).rgb;
        vec3 mapped = vec3(1.0) - exp(-hdrColor * u_Exposure);
        mapped = pow(mapped, vec3(1.0 / gamma));
        
        color = vec4(mapped, 1.0);
    }
    else
    {
        vec3 textureColor = texture(u_Diffuse, v_TexCoord).rgb;
        textureColor = pow(textureColor, vec3(1.0 / gamma));
        color = vec4(textureColor, 1.0);
    }
}