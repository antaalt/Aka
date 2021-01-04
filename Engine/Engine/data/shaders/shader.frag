#version 150

precision highp float;

in vec2 v_uv;

//uniform sampler2D u_Foreground;
uniform vec4 u_Color;

out vec4 FragColor;

void main()
{
	FragColor = u_Color;//texture(u_Foreground, v_uv);
	if (FragColor.a == 0.0)
		discard;
}
