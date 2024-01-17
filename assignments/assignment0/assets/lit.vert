#version 450
// Vertex attributes
layout (location = 0) in vec3 vPos; // vertex pos in model space
layout (location = 1) in vec3 vNormal; // Vertex position in model space

uniform mat4 _Model;
uniform mat4 _ViewProjection;


out vec3 Normal;

void main()
{
	Normal = vNormal;
	gl_Position = _ViewProjection * _Model * vec4(vPos, 1.0);
}