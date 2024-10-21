#version 450 core
in vec3 TexCoords;

out vec4 FragColor;

uniform samplerCube skybox;
uniform bool prefilter;
uniform float mipmap;

uniform float exposure;

void main()
{
    vec3 envColor = vec3(1.0);
    
    if (prefilter) envColor = textureLod(skybox, TexCoords, mipmap).rgb;
    else envColor = texture(skybox, TexCoords).rgb;

    //envColor = envColor / (envColor + vec3(1.0));
    envColor = vec3(1.0) - exp(-envColor * exposure);
    envColor = pow(envColor, vec3(1.0/2.2)); 

    FragColor = vec4(envColor, 1.0);
}