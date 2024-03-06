#version 450
// Vertex attributes
layout (location = 0) in vec2 vPos; // Quad Vertex Position
layout (location = 1) in vec2 vTexCoord; // Quad UV Coordinates


out vec2 TexCoords;
out vec4 FragPosLightSpace;

uniform mat4 LightSpaceMatrix;

void main()
{
	FragPosLightSpace = LightSpaceMatrix * vec4(vPos, 0.0,  1.0);
	TexCoords = vTexCoord;
	gl_Position = vec4(vPos, 0.0, 1.0);
}