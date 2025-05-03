#version 430 core
layout (location = 0) in vec3 vertPos;

out vec3 TexCoords;

layout (std140, binding = 0) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

void main()
{
    TexCoords = vertPos;
    vec4 pos = projection * mat4(mat3(view)) * vec4(vertPos, 1.0);
    gl_Position = pos.xyww;
} 