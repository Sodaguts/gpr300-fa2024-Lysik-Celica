#version 450

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;

void main() 
{
	float depth = texture(screenTexture, TexCoords).r;
	FragColor = vec4(vec3(depth), 1.0);
}