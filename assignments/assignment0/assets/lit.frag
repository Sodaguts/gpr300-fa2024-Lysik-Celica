#version 450

out vec4 FragColor; // the color of this fragment
//in vec3 Normal;
in Surface
{
	vec3 Normal;
	vec2 TexCoord;
}fs_in;
uniform sampler2D _MainTex; // 2D texture sampler
void main()
{
	//Shade with 0-1 Normal
	//FragColor = vec4(Normal * 0.5 + 0.5, 1.0);
	FragColor = texture(_MainTex, fs_in.TexCoord);
}