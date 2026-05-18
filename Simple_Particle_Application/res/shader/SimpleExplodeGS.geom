#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec3 v_LookAtCamera[];
in vec2 v_TexCoord[];

out vec2 v_InTexCoord;
out vec3 v_InNormal;
out vec3 v_InLookAtCamera;

void main() {
    for(int count = 0; count < 3; count++)
    {
        vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
        vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
        
        v_InNormal = normalize(cross(a, b));
        v_InLookAtCamera = v_LookAtCamera[count];
        v_InTexCoord = v_TexCoord[count];
        
        gl_Position = gl_in[count].gl_Position + vec4(v_InNormal, 0.0) * 2.0;
        EmitVertex();
    }
    
    EndPrimitive();
}