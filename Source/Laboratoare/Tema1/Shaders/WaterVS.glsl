#version 430

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;

// Uniform properties
uniform mat4 Model;

layout(location = 0) out int instance;

void main()
{
	instance = gl_InstanceID;
	gl_Position =  Model * vec4(v_position, 1);
}