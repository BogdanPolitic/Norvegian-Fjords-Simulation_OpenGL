#version 430

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;

layout(location = 0) out float rotHoriz;
layout(location = 1) out float rotVert;
layout(location = 2) out float size;

uniform mat4 Model;

struct Bubble {
	vec4 iniPos;
	vec4 pos;
	vec4 speed;
	vec4 rotation;
	vec4 size;
};

layout(std430, binding = 0) buffer particles {
	Bubble data[];
};

void main() {
	vec3 pos = data[gl_VertexID].pos.xyz;

	float dt = 0.1f;

	pos = vec3(pos.x + dt * data[gl_VertexID].speed.x, pos.y, pos.z);

	if (pos.x > 5.f) {
		pos = data[gl_VertexID].iniPos.xyz;
	}

	data[gl_VertexID].pos.xyz = pos;

	vec3 rot = data[gl_VertexID].rotation.xyz;
	float rotMagn = sqrt(pow(rot.x, 2) + pow(rot.y, 2) + pow(rot.z, 2));
	float rotMagnXZ = sqrt(pow(rot.x, 2) + pow(rot.z, 2));
	rotHoriz = asin(rot.y / rotMagn);
	rotVert = asin(rot.z / rotMagnXZ);
	size = data[gl_VertexID].size.x;

	gl_Position = Model * vec4(pos, 1);
}