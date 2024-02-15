#version 450

out vec4 FragColor; // the color of this fragment
//in vec3 Normal;
in Surface
{
	vec3 WorldPos;
	vec3 WorldNormal;
	vec2 TexCoord;
}fs_in;

struct PBRMaterial 
{
	sampler2D color;
	sampler2D metallic;
	sampler2D roughness;
	sampler2D occulsion;
	sampler2D specular;
};

uniform PBRMaterial material;
float PI = 3.14;
 //cache
vec3 col;
float mtl;
float rgh;
float spec;
float ao;

float NdotH;
float NdotV;
float NdotL;
float VdotH;
float VdotN;
float VdotL;
float LdotN;

uniform vec3 _EyePos;
uniform vec3 _LightPosition = vec3(0.0, 1.0, 0.0);
uniform vec3 _LightColor = vec3(1.0); //White Light
uniform vec3 _AmbientColor = vec3(0.3, 0.4, 0.46);

uniform vec3 _AmbientModifier = vec3(1.0, 1.0, 1.0);

uniform sampler2D _MainTex; // 2D texture sampler


//throwbridge-reitz model distribution
float D(float alpha)
{
	float numerator = pow(alpha, 2);
	float denominator = PI * pow((pow(NdotH,2) * (pow(alpha,2) - 1) + 1),2);
	return numerator / denominator;
}

//schlick-backmann
float G1(float alpha, float X)
{
	float k = alpha * 0.5;
	float denominator = X * (1.0 - k) + k;
	return X / denominator;
}

//smith model
float G(float alpha)
{
	return G1(alpha,LdotN) * G1(alpha, VdotL);
}

vec3 F(vec3 F0)
{
	return F0 + (vec3(1)-F0) * pow(1 - VdotH, 5);
}

vec3 PBR()
{
	vec3 F0 = vec3(0.4);
	F0 = col;

	//conservation of energy
	vec3 kS = vec3(F0);
	vec3 kD = (vec3(1.0) - kS) * (1.0 - mtl);

	// BDRF diffuse
	vec3 lambert = col / 5;

	//BDRF specular
	float alpha = pow(rgh,2);

	vec3 cookTorrenceNumerator = D(alpha) * G(alpha) * kS;
	float cookTorrenceDenominator = max(4.0 * VdotN * LdotN, 0.00001);
	vec3 cookTorrence = cookTorrenceNumerator / cookTorrenceDenominator;

	vec3 BDRF = (kD * lambert) + cookTorrence;

	return BDRF * _LightColor * LdotN;
}





void main()
{
	col = texture(material.color, fs_in.TexCoord).rgb;
	mtl = texture(material.metallic, fs_in.TexCoord).r;
	rgh = texture(material.roughness, fs_in.TexCoord).r;
	spec = texture(material.specular, fs_in.TexCoord).r;
	ao = texture(material.occulsion, fs_in.TexCoord).r;

	vec3 N = normalize(fs_in.WorldNormal);
	vec3 V = normalize(_EyePos);
	vec3 L = normalize(_LightPosition);
	vec3 H = normalize(V+L);

	//compute
	NdotH = max(dot(N,H), 0.0);
	NdotV = max(dot(N,V), 0.0);
	NdotL = max(dot(N,L), 0.0);
	VdotH = max(dot(V,H), 0.0);
	VdotN = max(dot(V,N), 0.0);
	LdotN = max(dot(L,N), 0.0);
	VdotL = max(dot(V,L), 0.0);

	//stylization
	vec3 reflectionDir = reflect(-L, N);
	float specAmount = pow(max(dot(V, reflectionDir),0.0),32);
	vec3 specular = spec * specAmount * _LightColor;

	vec3 finalColor = (_AmbientColor * col * ao) + PBR() + specular;

	finalColor = finalColor / (finalColor + vec3(1.0));
	finalColor = pow(finalColor, vec3(1.0/2.2)); 

	FragColor = vec4(finalColor,1);
}