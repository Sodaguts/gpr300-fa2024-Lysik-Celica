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
uniform float _Time;

//Light pointing straight down
uniform vec3 _EyePos;
uniform vec3 _WaterColor = vec3(0.0, 0.31, 0.85);

const float S1 = 0.9;
const float S2 = 0.1;

const float SCALE = 100;


void main()
{
	vec2 uv = fs_in.TexCoord * SCALE + vec2(sin(_Time));
	uv.x += sin(uv.y + _Time * 0.2) + sin(uv.y * 0.2 + _Time * 0.5) / 2.0;
	uv.y += sin(uv.x + _Time * 0.2) + sin(uv.x * 0.2 + _Time * 0.2) / 2.0;
	vec3 objectColor = texture(_MainTex, fs_in.TexCoord).rgb;
	vec4 smp1 = texture(_MainTex, uv * 1.0);
	vec4 smp2 = texture(_MainTex, uv * 1.5 + vec2(0.2));
	//FragColor = vec4(objectColor + _WaterColor,1.0);
	FragColor = vec4(_WaterColor + vec3(smp1 * S1 - smp2 * S2), 1.0);
}