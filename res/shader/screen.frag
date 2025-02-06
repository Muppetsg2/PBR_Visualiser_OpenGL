#version 450

in vec2 texCoord;

uniform sampler2D screenTexture;
uniform float pixelate;
uniform int mode;

out vec4 FragColor;

vec3 applyNegative(vec3 color) {
    return 1.0 - color;
}

vec3 applyGrayscale(vec3 color) {
    float average = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
    return vec3(average);
}

vec3 applyColor(vec3 color) {
    vec3 result = vec3(0.0);
	switch (mode) {
        case 0:
            result = color;
            break;
        case 1:
            result = applyNegative(color);
            break;
        case 2:
            result = applyGrayscale(color);
            break;
        default:
            result = color;
            break;
    }
    return result;
}

void main()
{
	vec2 resolution = gl_FragCoord.xy / texCoord.xy;

	vec2 sampleUVCoord = (floor(gl_FragCoord.xy / pixelate) * pixelate + 0.5) / resolution.xy;
	FragColor = vec4(applyColor(texture(screenTexture, sampleUVCoord.xy).rgb), 1.0);
}