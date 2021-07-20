#version 430
layout(location = 0) out vec4 out_color;

uniform vec3 _color;
uniform vec3 camera_position;
uniform sampler2D waterTexture;
uniform samplerCube skyboxTexture;


layout(location = 0) in vec2 texCoord;
layout(location = 1) in vec3 world_position;
layout(location = 2) in vec3 world_normal;

vec3 myReflect()
{
	//return reflect(world_position - camera_position, world_normal);
	return (world_position - camera_position) - 2 * dot(world_normal, world_position - camera_position) * world_normal;
}

void main()
{
	//out_color = vec4(_color, 0);
	//out_color = texture(waterTexture, texCoord);
	out_color = mix(texture(skyboxTexture, myReflect()), texture(waterTexture, texCoord), 0.4f);
}