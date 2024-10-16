#version 450 core  
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

layout (std140, binding = 0) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

out VS_OUT {
    vec2 TexCoords;
    vec3 WorldPos;
    mat3 TBN;
} vs_out;

uniform mat4 model;

void main()  
{  
    gl_Position = projection * view * model * vec4(position, 1.0);
    vs_out.TexCoords = texCoord;
    //vs_out.Normal = mat3(transpose(inverse(model))) * normal;
    vec3 T = normalize(vec3(model * vec4(tangent, 0.0)));
    vec3 B = normalize(vec3(model * vec4(bitangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(normal, 0.0)));
    vs_out.TBN = mat3(T, B, N);
    vs_out.WorldPos = vec3(model * vec4(position, 1.0));
}