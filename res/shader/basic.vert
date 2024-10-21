#version 450 core

#define POINT_LIGHTS 1

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
    vec3 TangentViewPos;
    vec3 TangentWorldPos;
    vec3 TangentLightPositions[POINT_LIGHTS];
    mat3 TBN;
} vs_out;

uniform vec3 camPos;

uniform mat4 model;

uniform vec3 lightPositions[POINT_LIGHTS];

void main()  
{  
    gl_Position = projection * view * model * vec4(position, 1.0);
    vs_out.TexCoords = texCoord;
    //vs_out.Normal = mat3(transpose(inverse(model))) * normal;
    vs_out.WorldPos = vec3(model * vec4(position, 1.0));

    vec3 T = normalize(vec3(model * vec4(tangent, 0.0)));
    vec3 B = normalize(vec3(model * vec4(bitangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(normal, 0.0)));
    vs_out.TBN = mat3(T, B, N);
    mat3 TBN = transpose(vs_out.TBN);

    vs_out.TangentViewPos = TBN * camPos;
    vs_out.TangentWorldPos = TBN * vs_out.WorldPos;

    for(int i = 0; i < POINT_LIGHTS; ++i)
    {
        vs_out.TangentLightPositions[i] = TBN * lightPositions[i];
    }
}