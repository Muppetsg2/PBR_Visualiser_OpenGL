#version 450

in vec2 texCoord;

uniform sampler2D screenTexture;

out vec4 FragColor;

void main()
{
	FragColor = vec4(texture(screenTexture, texCoord.xy).rgb, 1.0);
}