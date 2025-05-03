#version 430 core

out VS_OUT {
    vec2 TexCoords;
    vec3 WorldPos;
    vec3 TangentViewPos;
    vec3 TangentWorldPos;
    mat3 TBN;
} vs_out;

uniform vec3 camPos;

uniform mat4 model;

void main()  
{  
    float x = -1.0 + float((gl_VertexID & 1) << 2);
    float y = -1.0 + float((gl_VertexID & 2) << 1);
    gl_Position = vec4(x, y, 0.0 , 1.0);
    vs_out.TexCoords = vec2((x + 1.0) * 0.5, (1.0 - y) * 0.5);
    vs_out.WorldPos = vec3(model * vec4(vec3(0.0, y * 0.5, x * 0.5), 1.0));

    vec3 T = normalize(vec3(model * vec4(vec2(0.0), -1.0, 0.0)));
    vec3 B = normalize(vec3(model * vec4(0.0, 1.0, vec2(0.0))));
    vec3 N = normalize(vec3(model * vec4(1.0, vec3(0.0))));
    vs_out.TBN = mat3(T, B, N);
    mat3 TBN = transpose(vs_out.TBN);

    vs_out.TangentViewPos = TBN * camPos;
    vs_out.TangentWorldPos = TBN * vs_out.WorldPos;
}