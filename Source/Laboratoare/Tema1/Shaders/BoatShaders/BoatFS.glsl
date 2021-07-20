#version 430

layout(location = 0) in vec3 frag_normal;
layout(location = 1) in vec2 texture_coord;
layout(location = 2) in vec2 is_particle;

layout(location = 0) out vec4 out_color;

void main() {
	if (is_particle.x == 0)
		out_color = vec4(abs(frag_normal), 1);
	else
		out_color = vec4(1);
}