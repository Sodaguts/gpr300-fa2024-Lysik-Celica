#version 450

in vec2 TexCoords;
out vec4 FragColor;

uniform layout(location = 0) sampler2D gPosition;
uniform layout(location = 1) sampler2D gNormals;
uniform layout(location = 2) sampler2D gAlbedo;

uniform vec3 _EyePos;
uniform vec3 _LightDirection = vec3(0.0, -1.0, 0.0);
uniform vec3 _LightColor = vec3(1.0);
uniform vec3 _AmbientColor = vec3(0.3, 0.4, 0.46);

struct Material
{
	float Ka;
	float Kd;
	float Ks;
	float Shininess;
};
uniform Material _Material;

vec3 calculateLighting(vec3 position, vec3 normal, vec3 albedo)
{
	vec3 toLight = -_LightDirection;
	float diffuseFactor = max(dot(normal, toLight), 1.0);
	vec3 toEye = normalize(_EyePos - position);
	vec3 h = normalize(toLight + toEye);
	float specularFactor = pow(max(dot(normal, h), 0.0),_Material.Shininess);
	vec3 lightColor = (_Material.Kd * diffuseFactor + _Material.Ks * specularFactor) * _LightColor;
	lightColor += _AmbientColor  * _Material.Ka;
	return albedo * lightColor;
}

void main() 
{
	vec3 Position = texture(gPosition, TexCoords).rgb;
	vec3 Normal = texture(gNormals, TexCoords).rgb;
	vec3 Albedo = texture(gAlbedo, TexCoords).rgb;
	
	FragColor = vec4(calculateLighting(Position, Normal, Albedo),1.0);
}