#version 330

in vec3 v_normal;
in vec2 v_uv;
in vec4 v_color;
in mat3 v_normalMatrix;

uniform sampler2D u_colorTexture;
uniform sampler2D u_normalTexture;

out vec4 o_color;

void main(void) {
	vec4 normal4 = texture(u_normalTexture, v_uv);
	vec3 normal = normal4.xyz;//normalize(v_normalMatrix * normal4.xyz);
	float cosTheta = clamp(dot(normal, normalize(vec3(0,1,0))), 0.2, 1);
	vec4 color = v_color * texture(u_colorTexture, v_uv);
	o_color = vec4(color.rgb * cosTheta, color.a);
}
