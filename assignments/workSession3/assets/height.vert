#version 450
layout (location = 0) in vec3 vPos; // vertex pos in model space
layout (location = 1) in vec3 vNormal; // Vertex position in model space
layout (location = 2) in vec2 vTexCoord; // Vertex texture coordinate (UV)

uniform mat4 _Model; // Model -> World
uniform mat4 _ViewProjection; // Combined View -> Projection

uniform sampler2D _heightMap;
uniform float _heightScalar;

out vec3 Normal;
out float o_scalar;
//This whole block will be passed to the next shader stage
out Surface
{
	vec3 WorldPos;
	vec3 WorldNormal;
	vec2 TexCoord;
}vs_out;

void main()
{
	//Transform vertex Position to world space
	vs_out.WorldPos = vec3(_Model * vec4(vPos, 1.0));
	//Transform vert normal to world space using normal matrix
	vs_out.WorldNormal = transpose(inverse(mat3(_Model))) * vNormal;

	float scalar = texture(_heightMap, vTexCoord).r;
	o_scalar = scalar;

	vec3 pos = vPos;
	pos.y = scalar * _heightScalar;
	//vs_out.Normal = vNormal;
	vs_out.TexCoord = vTexCoord;
	gl_Position = _ViewProjection * _Model * vec4(pos, 1.0);
}