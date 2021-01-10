#version 330

precision highp float;

// triangle strip (reversed N shaped)
const vec2 positions[4] = vec2[4](
	vec2(-1.0,1.0), vec2(-1.0,-1.0),
	vec2(1.0,1.0), vec2(1.0,-1.0)
);

out vec2 uv;

uniform mat4 projection;
uniform mat4 model;

void main()
{
	gl_Position = projection * model * vec4(positions[gl_VertexID], 0.0, 1.0);
	uv = positions[gl_VertexID];
}
