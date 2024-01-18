#version 450

out vec4 FragColor; // the color of this fragment
//in vec3 Normal;
in Surface
{
	vec3 WorldPos;
	vec3 WorldNormal;
	vec2 TexCoord;
}fs_in;

uniform sampler2D _MainTex; // 2D texture sampler

//Light pointing straight down
uniform vec3 _LightDirection = vec3(0.0, -1.0, 0.0);
uniform vec3 _LightColor = vec3(1.0); //White Light

void main()
{
	//Shade with 0-1 Normal
	//FragColor = vec4(Normal * 0.5 + 0.5, 1.0);
	vec3 normal = normalize(fs_in.WorldNormal);
	vec3 toLight = -_LightDirection;
	float diffuseFactor = max(dot(normal,toLight), 0.0);
	vec3 diffuseColor = _LightColor * diffuseFactor;
	vec3 objectColor = texture(_MainTex, fs_in.TexCoord).rgb;
	FragColor = vec4(objectColor * diffuseColor, 1.0);
}