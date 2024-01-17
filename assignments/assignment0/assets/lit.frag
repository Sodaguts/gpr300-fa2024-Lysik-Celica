#version 450

out vec4 FragColor; // the color of this fragment
in vec3 Normal;

void main()
{
	//Shade with 0-1 Normal
	FragColor = vec4(Normal * 0.5 + 0.5, 1.0);
}