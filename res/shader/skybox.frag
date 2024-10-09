#version 450 core
in vec3 TexCoords;

out vec4 FragColor;

uniform samplerCube skybox;

void main()
{    
    FragColor = vec4(pow(texture(skybox, TexCoords).rgb, vec3(1.0/2.2)), 1.0);
}