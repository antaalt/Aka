#version 330

in vec3 v_normal;
in vec2 v_uv;
in vec4 v_color;

uniform sampler2D u_texture;

out vec4 o_color;

void main(void) {
	float cosTheta = clamp(dot(v_normal, normalize(vec3(0,1,0))), 0.2, 1);
	vec4 color = v_color * texture(u_texture, v_uv);
	o_color = vec4(color.rgb * cosTheta, color.a);
}
