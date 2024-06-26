#version 450
out vec4 FragColor;

in VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
	vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D MainTexture;
uniform sampler2D shadowMap;

uniform float texelMod = 3.0f;

uniform vec3 lightPos = vec3(0.0,1.0,0.0);
uniform vec3 viewPos;

float ShadowCalculation(vec4 fragPosLightSpace)
{
	//perform perspective divide
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	//transform to [0,1] range
	projCoords = projCoords * 0.5 + 0.5;
	//get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
	float closestDepth = texture(shadowMap, projCoords.xy).r;
	//get depth of current fragment from light's perspective
	float currentDepth = projCoords.z;
	//check whether current frag pos is in shadow

	vec3 normal = normalize(fs_in.Normal);
	vec3 lightDir = normalize(lightPos - fs_in.FragPos);

	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

	//PCF
	vec2 texelSize = texelMod/ textureSize(shadowMap, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <=1; ++y)
		{
			float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x,y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}	
	}
	shadow /= 9.0;

	if(projCoords.z > 1.0)
	{
		shadow = 0.0;
	}

	return shadow;
}

struct Material
{
	float Ka;
	float Kd;
	float Ks;
	float Shininess;
};
uniform Material _Material;

void main()
{
	vec3 color = texture(MainTexture, fs_in.TexCoords).rgb;
	vec3 normal = normalize(fs_in.Normal);
	vec3 lightColor = vec3(1.0);

	vec3 ambient = vec3(0.3, 0.4, 0.46)* lightColor;
	//vec3 lightDir = normalize(lightPos - fs_in.FragPos);
	vec3 lightDir = vec3(0.0, -1.0, 0.0);
	vec3 toLight = -lightDir;

	float diff = max(dot(normal, toLight), 0.0);
	vec3 diffuse = diff * lightColor;

	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	float spec = 0.0;
	vec3 halfwayDir = normalize(toLight + viewDir);
	spec = pow(max(dot(normal, halfwayDir), 0.0),_Material.Shininess);
	vec3 specular = spec * lightColor;

	float shadow = ShadowCalculation(fs_in.FragPosLightSpace);


	vec3 lighting = (ambient + (1.0 - shadow) * ((diffuse*_Material.Kd)+(specular*_Material.Ks))) * color;

	FragColor = vec4(lighting, 1.0);
}