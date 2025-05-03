#version 430 core

#define M_PI 3.1415926535897932384626433832795

const float MAX_REFLECTION_LOD = 4.0;

in VS_OUT {
    vec2 TexCoords;
    vec3 WorldPos;
    vec3 TangentViewPos;
    vec3 TangentWorldPos;
    mat3 TBN;
} fs_in;

out vec4 FragColor;

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;
uniform float skyboxExposure;
uniform float skyboxColorIntensity;

uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D displacementMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

uniform float height_scale;
uniform float exposure;
uniform float colorIntensity;

uniform vec3 camPos;

vec3 ApplyExposureAndIntensity(vec3 color, float e, float i)
{
    vec3 ret = color;
    // Exposure
    ret = vec3(1.0) - exp(-ret * e);
    // Color Intensity
    ret *= i;
    return ret;
}

vec3 GetNormalFromNormalMap(sampler2D map, vec2 coords)
{
    vec3 normal = texture(map, coords).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    normal = normalize(fs_in.TBN * normal);
    return normal;
}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
    const float minLayers = 8.0;
    const float maxLayers = 32.0;
    // number of depth layers
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy / viewDir.z * height_scale; 
    vec2 deltaTexCoords = P / numLayers;

    // get initial values
    vec2  currentTexCoords = texCoords;
    float currentDepthMapValue = texture(displacementMap, currentTexCoords).r;

    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(displacementMap, currentTexCoords).r;  
        // get depth of next layer
        currentLayerDepth += layerDepth;  
    }

    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(displacementMap, prevTexCoords).r - currentLayerDepth + layerDepth;

    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;  
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = M_PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 PrefilteredColor(vec3 R, float roughness)
{
    return ApplyExposureAndIntensity(textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb, skyboxExposure, skyboxColorIntensity);
}

vec3 IrradianceColor(vec3 N)
{
    return ApplyExposureAndIntensity(texture(irradianceMap, N).rgb, skyboxExposure, skyboxColorIntensity);
}

void main() 
{
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentWorldPos);

    vec2 texCoords = ParallaxMapping(fs_in.TexCoords, viewDir);

    vec3 albedo = texture(albedoMap, texCoords).rgb;
    vec3 normal = GetNormalFromNormalMap(normalMap, texCoords);
    float metallic = texture(metallicMap, texCoords).r;
    float roughness = texture(roughnessMap, texCoords).r;
    float ao = texture(aoMap, texCoords).r;

    vec3 N = normalize(normal);
    vec3 V = normalize(camPos - fs_in.WorldPos);
    vec3 R = reflect(-V, N);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    vec3 irradiance = IrradianceColor(N);
    vec3 diffuse = irradiance * albedo;

    vec3 prefilteredColor = PrefilteredColor(R, roughness);
    vec2 envBRDF  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);

    vec3 color = (kD * diffuse + specular) * ao;

    color = ApplyExposureAndIntensity(color, exposure, colorIntensity);
    // Gamma Correction
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0);
}