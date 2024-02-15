#version 450
// Vertex attributes
layout (location = 0) in vec3 vPos; // vertex pos in model space
layout (location = 1) in vec3 vNormal; // Vertex position in model space
layout (location = 2) in vec2 vTexCoord; // Vertex texture coordinate (UV)

uniform mat4 _Model; // Model -> World
uniform mat4 _ViewProjection; // Combined View -> Projection

uniform float waveStrength = 0.5;
uniform float waveScale = 0.1;
uniform float _Time;

out vec3 Normal;
//This whole block will be passed to the next shader stage
out Surface
{
	vec3 WorldPos;
	vec3 WorldNormal;
	vec2 TexCoord;
}vs_out;

float calculateSurface(float _x, float _z)
{
	float y = 0.0;
	y += (sin(_x * 0.2 / waveScale + _Time * 0.2) + sin(_x * 0.8 / waveScale + _Time * 0.4) + sin(_x * 0.6 / waveScale + _Time * 0.6)) / 3.0;
	y += (sin(_z * 0.1 / waveScale + _Time * 0.1) + sin(_z * 0.3 / waveScale + _Time * 0.3) + sin(_z * 0.2 / waveScale + _Time * 0.2)) / 3.0;
	return y;
}

void main()
{
	vec3 position = vPos;
	position.y = calculateSurface(position.x, position.z) * waveStrength;

	vs_out.WorldPos = vec3(_Model * vec4(vPos, 1.0));

	vs_out.WorldNormal = transpose(inverse(mat3(_Model))) * vNormal;

	vs_out.TexCoord = vTexCoord * 20;
	gl_Position = _ViewProjection * _Model * vec4(position, 1.0);
}