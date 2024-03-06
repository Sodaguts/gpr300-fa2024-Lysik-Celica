#version 450

in vec2 TexCoords;
out vec4 FragColor;

in vec4 FragPosLightSpace;

uniform layout(location = 0) sampler2D gPosition;
uniform layout(location = 1) sampler2D gNormals;
uniform layout(location = 2) sampler2D gAlbedo;
uniform layout(location = 3) sampler2D gDepth; // shadow map

uniform vec3 _EyePos;
uniform vec3 _LightDirection = vec3(0.0, -1.0, 0.0);
uniform vec3 _LightColor = vec3(1.0);
uniform vec3 _AmbientColor = vec3(0.3, 0.4, 0.46);

uniform vec3 _LightPos = vec3(0.0, 1.0, 0.0);
uniform float texelMod = 3.0f;

struct Material
{
	float Ka;
	float Kd;
	float Ks;
	float Shininess;
};
uniform Material _Material;

float ShadowCalculation(vec4 fragPosLightSpace)
{
	
	vec3 in_normals = texture(gNormals, TexCoords).rgb;
	vec3 in_position = texture(gPosition, TexCoords).rgb;

	//perform perspective divide
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	//transform 0,1 range
	projCoords = projCoords * 0.5 + 0.5;

	float closestDepth = texture(gDepth, projCoords.xy).r;
	float currentDepth = projCoords.z;

	vec3 normal = normalize(in_normals);
	vec3 lightDir = normalize(_LightPos - in_position);

	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
	
	//PCF
	vec2 texelSize = texelMod / textureSize(gDepth, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(gDepth, projCoords.xy + vec2(x,y) * texelSize).r;
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

struct PointLight
{
	vec3 position;
	float radius;
	vec4 color;
};

#define MAX_POINT_LIGHTS 64
uniform PointLight _MainLight;
uniform PointLight _PointLights[MAX_POINT_LIGHTS];

float attenuateLinear(float _distance, float _radius)
{
	return clamp(((_radius - _distance)/_radius), 0.0,1.0);
}

vec3 calcPointLight(PointLight light, vec3 _normal)
{
	vec3 diff = light.position - texture(gPosition,TexCoords).rgb;
	//direction toward light position
	vec3 toLight = normalize(diff);
	//TODO: usual blinn-phong calculations for diffuse + specular
	float diffuseFactor = max(dot(_normal, toLight), 1.0);
	vec3 toEye = normalize(_EyePos - texture(gPosition,TexCoords).rgb);

	vec3 h = normalize(toLight + toEye);
	float specularFactor = pow(max(dot(_normal, h), 0.0),0.01);
	vec4 lightColor = (diffuseFactor + specularFactor) * light.color;
	float d = length(diff);
	lightColor *= attenuateLinear(d, light.radius);
	return lightColor.rgb; //changing it to a vec3 but idk why this function is a vec3 when color is a vec4 in the first place
}


vec3 calculateLighting(vec3 position, vec3 normal, vec3 albedo)
{
	vec3 toLight = -_LightDirection;
	float diffuseFactor = max(dot(normal, toLight), 1.0);
	vec3 toEye = normalize(_EyePos - position);

	vec3 h = normalize(toLight + toEye);
	float specularFactor = pow(max(dot(normal, h), 0.0),_Material.Shininess);

	//float shadow = ShadowCalculation(FragPosLightSpace);

	vec3 lightColor = (_Material.Kd * diffuseFactor + _Material.Ks * specularFactor) * _LightColor;
	lightColor += (_AmbientColor)  * _Material.Ka; //For shadows you would take the ambient color and add (1.0 - shadow)
	return albedo * lightColor;
}

void main() 
{
	vec3 Position = texture(gPosition, TexCoords).rgb;
	vec3 Normal = texture(gNormals, TexCoords).rgb;
	vec3 Albedo = texture(gAlbedo, TexCoords).rgb;

	//Point Light stuff

	vec3 totalLight = vec3(0);
	totalLight += calculateLighting(Position,Normal,Albedo);
	for(int i = 0; i<MAX_POINT_LIGHTS; i++)
	{
		totalLight += calcPointLight(_PointLights[i], Normal);
	}
	
	
	FragColor = vec4(calculateLighting(Position, Normal, Albedo),1.0);
}