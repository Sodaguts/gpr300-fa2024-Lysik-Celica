#version 450

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;

void main() 
{
	vec3 rgb = texture(screenTexture, TexCoords).rgb;
	vec4 position = texture(screenTexture, TexCoords);
	FragColor = vec4(rgb, 1.0);
}