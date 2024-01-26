#version 450
// Vertex attributes
layout (location = 0) in vec2 vPos; // Quad Vertex Position
layout (location = 2) in vec2 vTexCoord; // Quad UV Coordinates

out vec2 TexCoords;

void main()
{
	TexCoords = vTexCoord;
	gl_Position = vec4(vPos, 0.0, 1.0);
}