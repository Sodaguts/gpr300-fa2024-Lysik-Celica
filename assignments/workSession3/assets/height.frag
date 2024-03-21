#version 450

out vec4 FragColor; // the color of this fragment
//in vec3 Normal;
in Surface
{
	vec3 WorldPos;
	vec3 WorldNormal;
	vec2 TexCoord;
}fs_in;

in float o_scalar;
uniform sampler2D _MainTex; // 2D texture sampler
uniform float _OtherColor;


void main()
{
//TODO: change island color based on user input
	vec3 normal = normalize(fs_in.WorldNormal);
	vec3 objectColor = texture(_MainTex, fs_in.TexCoord).rgb;
	vec3 otherColor = vec3(0.0, 0.0, 1.0);
	otherColor.y = o_scalar;
	FragColor = vec4(otherColor, 1.0);
}