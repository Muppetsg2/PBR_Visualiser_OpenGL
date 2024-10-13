#version 450 core  
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 normal;

layout (std140, binding = 0) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

out VS_OUT {
	vec2 texCoord;
	vec3 normal;
	vec3 fragPos;
} vs_out;

uniform mat4 model;

void main()  
{  
  gl_Position = projection * view * model * vec4(position, 1.0);
  vs_out.texCoord = texCoord;
  vs_out.normal = mat3(transpose(inverse(model))) * normal;
  vs_out.fragPos = vec3(model * vec4(position, 1.0));
}