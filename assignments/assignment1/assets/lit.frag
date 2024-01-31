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
uniform vec3 _EyePos;
uniform vec3 _LightDirection = vec3(0.0, -1.0, 0.0);
uniform vec3 _LightColor = vec3(1.0); //White Light
uniform vec3 _AmbientColor = vec3(0.3, 0.4, 0.46);

uniform vec3 _LightPos = vec3(0.0,1.0,0.0);

//uniform vec3 _AmbientModifier = vec3(1.0, 1.0, 1.0);
uniform int _isInverted = 0;

//gamma correction
uniform int _gamma = 0;

struct Material
{
	float Ka; // Ambient coefficient (0-1)
	float Kd; // Diffuse coefficient (0-1)
	float Ks; // Specular coefficient (0-1)
	float Shininess; // Affects size of specular highlight
};
uniform Material _Material;

void main()
{
	vec3 normal = normalize(fs_in.WorldNormal);
	vec3 toLight = -_LightDirection;
	float diffuseFactor = max(dot(normal,toLight), 0.0);
	vec3 toEye = normalize(_EyePos - fs_in.WorldPos);
	//Blinn-phong uses half angle
	vec3 h = normalize(toLight + toEye);
	float specularFactor = pow(max(dot(normal, h),0.0),_Material.Shininess);
	

	float max_distance = 1.5;
	float adistance = length(_LightPos - fs_in.WorldPos);
	float attenuation;
	if (_gamma == 1)
	{
		attenuation = 1.0/(adistance*adistance);
	}
	else
	{
		attenuation = 1.0/(adistance);
	}
	diffuseFactor *= attenuation;
	specularFactor*= attenuation;

	//Combination of specular and diffuse reflection
	vec3 lightColor = ((_Material.Kd * diffuseFactor) + (_Material.Ks * specularFactor)) * _LightColor;
	lightColor += (_AmbientColor) * _Material.Ka;
	vec3 objectColor = texture(_MainTex, fs_in.TexCoord).rgb;

	vec3 totalColor = objectColor * lightColor;
	vec3 vecOne = vec3(1,1,1);
	if(_isInverted == 1)
	{
		if(_gamma == 1)
		{
			FragColor.rgb = pow(totalColor, vec3(1.0/2.2));
		}
		FragColor = vec4(vecOne - totalColor, 1.0);
	}
	else
	{
		if(_gamma ==1)
		{
			FragColor.rgb = pow(totalColor, vec3(1.0/2.2));
		}
		FragColor = vec4(totalColor,1.0);
	}
}