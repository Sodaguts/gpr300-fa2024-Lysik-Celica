#version 450

out vec4 FragColor; // the color of this fragment
in vec2 UV;

uniform sampler2D _Texture;
uniform float cutoffValue = 0.5;

void main(){
	vec4 bgTexture = vec4(texture(_Texture, UV).r, 0.0, 0.0, 1.0);
	if(texture(_Texture,UV).r >= cutoffValue)
	{
		discard;
	}
	FragColor = vec4(1.0, 0.0, 1.0, 1.0);
	
}