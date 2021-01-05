#version 330

precision highp float;

// Triangle strip (reversed N shaped)
// Use glDrawArrays(GL_TRIANGLE_STRIP, 0, 4)
vec2 positions[4] = vec2[4](
	vec2(-1.0,1.0), vec2(-1.0,-1.0),
	vec2(1.0,1.0), vec2(1.0,-1.0)
);

out vec2 v_uv;

void main()
{
	gl_Position = vec4(positions[gl_VertexID], 0.0, 1.0);
	v_uv = gl_Position.xy * 0.5 + 0.5;
}
