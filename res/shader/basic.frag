#version 450 core

in VS_OUT {
    vec2 texCoord;
    vec3 normal;
    vec3 fragPos;
} fs_in;

out vec4 FragColor;

/*
struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    bool on;
};
*/

//uniform DirLight dirLight;
//uniform vec3 viewPos;

//vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);

void main() 
{
    vec3 norm = normalize(fs_in.normal);
    //vec3 viewDir = normalize(viewPos - fs_in.fragPos);
    vec3 result = vec3(0.5, 0.0, 0.5);

    // Directional Light
    //result += dirLight.on ? CalcDirLight(dirLight, norm, viewDir) : vec3(0.0, 0.0, 0.0);

    FragColor = vec4(result, 1.0);
}

/*
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // swiatlo rozproszone
    float diff = max(dot(normal, lightDir), 0.0);
    // swiatlo lustrzane
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(viewDir, halfwayDir), 0.0), material.shininess);
    // polacz wyniki
    vec3 ambient = light.ambient * (material.hasDiffuse ? vec3(texture(material.diffuse, fs_in.texCoord)) : material.diffuseColor);
    vec3 diffuse = light.diffuse * diff * (material.hasDiffuse ? vec3(texture(material.diffuse, fs_in.texCoord)) : material.diffuseColor);
    vec3 specular = light.specular * spec * (material.hasSpecular ? vec3(texture(material.specular, fs_in.texCoord)) : material.specularColor);
    return (ambient + diffuse + specular);
}
*/