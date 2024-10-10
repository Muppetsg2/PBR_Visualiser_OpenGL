#version 450 core

out vec2 TexCoords;

void main(void) 
{
    float x = float((gl_VertexID & 1) << 2);
    float y = float((gl_VertexID & 2) << 1);
    TexCoords.x = x * 0.5;
    TexCoords.y = y * 0.5;
    gl_Position = vec4(x - 1.0, y - 1.0, 0, 1);
}