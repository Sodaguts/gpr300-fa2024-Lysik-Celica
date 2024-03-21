#version 450

layout(location = 0) out vec4 FragColor;

in Surface
{
	vec3 WorldPos;
	vec3 WorldNormal;
	vec2 TexCoord;
}fs_in;

uniform sampler2D _gWorldPos;
uniform sampler2D _gWorldNormal;
uniform sampler2D _gAlbedo;




void main()
{
	vec3 worldPos = texture(_gWorldPos, fs_in.TexCoord).rgb;
	vec3 albedo = texture(_gAlbedo, fs_in.TexCoord).rgb;
	vec3 normal = texture(_gWorldNormal, fs_in.TexCoord).rgb;



	// Calculate Lighting...
	vec3 objectColor = worldPos + albedo + normal;
	vec3 lightColor = vec3(1.0,1.0,1.0);
	FragColor = vec4(objectColor * lightColor, 1.0);
}