#version 330 core

uniform sampler2D diffuse;

in  vec2 v_TexCoord;
out vec4 color;

const float offsetH = 1.0 / 1280.0;
const float offsetV = 1.0 / 720.0;

void main()
{
	vec2 offsets[9] = vec2[](
	vec2(-offsetH,  offsetV), // 左上
	vec2( 0.0f,    offsetV), // 正上
	vec2( offsetH,  offsetV), // 右上
	vec2(-offsetH,  0.0f),   // 左
	vec2( 0.0f,    0.0f),   // 中
	vec2( offsetH,  0.0f),   // 右
	vec2(-offsetH, -offsetV), // 左下
	vec2( 0.0f,   -offsetV), // 正下
	vec2( offsetH, -offsetV)  // 右下
	);

	// 高斯模糊
	float kernel[9] = float[](
	1.0 / 16, 2.0 / 16, 1.0 / 16,
	2.0 / 16, 4.0 / 16, 2.0 / 16,
	1.0 / 16, 2.0 / 16, 1.0 / 16
	);

	vec3 sampleTex[9];
	for(int i = 0; i < 9; i++)
	{
		sampleTex[i] = vec3(texture(diffuse, v_TexCoord.st + offsets[i]));
	}
	vec3 col = vec3(0.0);
	for(int i = 0; i < 9; i++)
	col += sampleTex[i] * kernel[i];

//	color = vec4(col, 1.0);
	color = texture(diffuse, v_TexCoord);
}