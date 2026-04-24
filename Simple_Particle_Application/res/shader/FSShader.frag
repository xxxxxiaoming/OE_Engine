#version 330 core

uniform sampler2D u_Texture[4];

in float v_TexSlot;
in vec2 v_TexCoord;
out vec4 color;
in vec4 v_Color;

void main() {
    int slot = int(v_TexSlot);
    color = texture(u_Texture[slot], v_TexCoord);
//    color = v_Color;

    /* 注意，这里是将纹理上，对应坐标的颜色RGB完完整整取出来，不会管Alpha通道的值的，就算Alpha是0，也会被完整取出来，写入到frame buffer中 */
    /* 所以要么在OpenGL开启Blend，要么在这里，自行过滤： */
    //if(color.a < 0.1)
    //{
        //discard;
    //}
};