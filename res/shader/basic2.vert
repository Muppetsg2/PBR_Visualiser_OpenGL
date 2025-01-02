#version 450 core

layout (location = 0) in vec3 position;

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
    //gl_Position = projection * view * model * vec4(position, 1.0);
    gl_Position = vec4(position.z * 2.0, position.y * 2.0, 0.0 , 1.0);
    // position = vec3(0.0, y / height, x / width) - vec3(0.0, 0.5, 0.5)
    vs_out.TexCoords = vec2(position.z + 0.5, -(position.y - 0.5));
    vs_out.WorldPos = vec3(model * vec4(position, 1.0));

    vec3 T = normalize(vec3(model * vec4(vec2(0.0), -1.0, 0.0)));
    vec3 B = normalize(vec3(model * vec4(0.0, 1.0, vec2(0.0))));
    vec3 N = normalize(vec3(model * vec4(1.0, vec3(0.0))));
    vs_out.TBN = mat3(T, B, N);
    mat3 TBN = transpose(vs_out.TBN);

    vs_out.TangentViewPos = TBN * camPos;
    vs_out.TangentWorldPos = TBN * vs_out.WorldPos;
}