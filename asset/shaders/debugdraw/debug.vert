#version 450
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec4 a_color;
layout(location = 0) out vec3 v_position;
layout(location = 1) out vec4 v_color;
layout(push_constant) uniform constants {
	mat4 mvp;
} u_push;
void main(void) {
	v_position = a_position;
	gl_Position = u_push.mvp * vec4(v_position, 1.0);
	v_color = a_color;
}