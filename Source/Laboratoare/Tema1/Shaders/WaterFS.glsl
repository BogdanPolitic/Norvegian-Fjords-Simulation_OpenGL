#version 430
layout(location = 0) out vec4 out_color;

uniform vec3 _color;
uniform sampler2D waterTexture;
layout(location = 0) in vec2 texCoord;

void main()
{
	//out_color = vec4(_color, 0);
	out_color = texture2D(waterTexture, texCoord);
}