#version 330 core

in vec3 v_Normal;
in vec3 v_LookAtCamera;
in vec2 v_TexCoord;

out vec4 Color;

uniform samplerCube u_CubeMap;

void main()
{
//    vec3 ReflectVector = reflect(-v_LookAtCamera, normalize(v_Normal));
      vec3 ReflectVector = refract(-v_LookAtCamera, normalize(v_Normal), 1.0 / 2.42);
      Color = vec4(texture(u_CubeMap, ReflectVector).rgb, 1.0);
}