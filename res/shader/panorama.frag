#version 430 core

#define M_PI 3.1415926535897932384626433832795

in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D u_panorama;
uniform int u_currentFace;

vec3 uvToXYZ(int face, vec2 uv)
{
	if(face == 0)
		return vec3(     1.0,   uv.y,    -uv.x);

	else if(face == 1)
		return vec3(    -1.0,   uv.y,     uv.x);

	else if(face == 2)
		return vec3(   +uv.x,   -1.0,    +uv.y);

	else if(face == 3)
		return vec3(   +uv.x,    1.0,    -uv.y);

	else if(face == 4)
		return vec3(   +uv.x,   uv.y,      1.0);

	else //if(face == 5)
	{	return vec3(    -uv.x,  +uv.y,     -1.0);}
}

vec2 dirToUV(vec3 dir)
{
	return vec2(
		0.5 + 0.5 * atan(dir.z, dir.x) / M_PI,
		1.0 - acos(dir.y) / M_PI);
}

vec3 panoramaToCubeMap(int face, vec2 texCoord)
{
	vec2 texCoordNew = texCoord * 2.0 - 1.0; //< mapping vom 0,1 to -1,1 coords
	vec3 scan = uvToXYZ(face, texCoordNew); 
	vec3 direction = normalize(scan);
	vec2 src = dirToUV(direction);

	return texture(u_panorama, src).rgb; //< get the color from the panorama
}

void main()
{    
    FragColor = vec4(panoramaToCubeMap(u_currentFace, TexCoords).rgb, 1.0);
}