#version 430

uniform sampler2D tex;

layout(location = 0) in vec2 texCoord;
layout(location = 0) out vec4 out_color;

void main() {
	vec3 color = texture2D(tex, texCoord).xyz;
	out_color = vec4(color, 1.f);
}