#version 430 core
layout (location = 0) in vec3 position;

out vec2 TexCoords;

float map(float x, float currStart, float currEnd, float expectedStart, float expectedEnd) 
{
    return expectedStart + ((expectedEnd - expectedStart) / (currEnd - currStart)) * (x - currStart);
}

void main()  
{  
    gl_Position = vec4(map(position.z, -.5, .5, -1.0, 1.0), map(position.x, -.5, .5, -1.0, 1.0), 0.0, 1.0);
    TexCoords = vec2(map(position.z, -0.5, 0.5, 0.0, 1.0), map(position.x, 0.5, -0.5, 1.0, 0.0));
}