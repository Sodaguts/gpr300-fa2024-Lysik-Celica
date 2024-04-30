#version 450

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;
const float offset = 1.0 / 300.0;
uniform vec3 outlineColor;

void main() 
{
	vec2 offsets[9] = vec2[](
		vec2(-offset, offset),
		vec2(0.0f, offset),
		vec2(offset, offset),
		vec2(-offset, 0.0f),
		vec2(0.0f, 0.0f),
		vec2(offset, 0.0f),
		vec2(-offset, -offset),
		vec2(0.0f, -offset),
		vec2(offset, -offset)
	);

	float kernel[9] = float[](
		 1,  1,  1,
		 1,  -9,  1,
		 1,  1,  1
	);

	vec3 sampleTex[9];
	for(int i = 0; i < 9; i++)
	{
		sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
	}
	vec3 col = vec3(0.0);
	for(int i = 0; i< 9; i++)
	{
		col += sampleTex[i] * kernel[i];
	}

	vec3 rgb = texture(screenTexture, TexCoords).rgb;
	vec3 finalColor;
	normalize(col);
	if(col.r >= -0 && col.g >= 0 && col.b >= 0)
	{
		finalColor = outlineColor;
	}
	else
	{
		finalColor = rgb;
	}

	vec4 position = texture(screenTexture, TexCoords);
	FragColor = vec4(finalColor, 1.0);
}