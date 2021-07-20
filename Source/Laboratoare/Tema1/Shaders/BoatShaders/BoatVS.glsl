#version 430

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;

layout(location = 0) out vec3 frag_normal;
layout(location = 1) out float rotHoriz;
layout(location = 2) out float rotVert;
layout(location = 3) out float size;
layout(location = 4) out vec4 bubbleGlobalCenter;
layout(location = 5) out mat4 boatModel;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

uniform vec3 main_control_p1, main_control_p2, main_control_p3, main_control_p4;
uniform vec3 front_control_p1, front_control_p2, front_control_p3, front_control_p4;
uniform vec3 left_control_p2, left_control_p3, left_control_p4;
uniform vec3 right_control_p2, right_control_p3, right_control_p4;
uniform int no_inst;
uniform int no_gen;
uniform float branch_width;
uniform float elapsedTime;
uniform float main_progress;
uniform int path;
uniform int orientation;

vec3 left_control_p1, right_control_p1;

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

vec2 vec3To2(vec3 vect) {
	return vec2(vect.x, vect.z);
}

vec3 vec2To3(vec2 vect, float height) {
	return vec3(vect.x, height, vect.y);
}

float slope(vec2 p1, vec2 p2) {
	return (p2.y - p1.y) / (p2.x - p1.x);
}

vec3 bezier_main(float t)
{
	return main_control_p1 * pow((1 - t), 3) 
			+ main_control_p2 * 3 * t * pow((1 - t), 2) 
			+ main_control_p3 * 3 * pow(t, 2) * (1 - t) 
			+ main_control_p4 * pow(t, 3);
}

vec3 bezier_front(float t)
{
	return front_control_p1 * pow((1 - t), 3) 
			+ front_control_p2 * 3 * t * pow((1 - t), 2) 
			+ front_control_p3 * 3 * pow(t, 2) * (1 - t) 
			+ front_control_p4 * pow(t, 3);
}

vec3 bezier_left(float t) {
	return left_control_p1 * pow((1 - t), 3) 
			+ left_control_p2 * 3 * t * pow((1 - t), 2) 
			+ left_control_p3 * 3 * pow(t, 2) * (1 - t) 
			+ left_control_p4 * pow(t, 3);
}

vec3 bezier_right(float t) {
	return right_control_p1 * pow((1 - t), 3) 
			+ right_control_p2 * 3 * t * pow((1 - t), 2) 
			+ right_control_p3 * 3 * pow(t, 2) * (1 - t) 
			+ right_control_p4 * pow(t, 3);
}

vec3 bezier(float t) {
	switch (path) {
		case 0:
			return bezier_main(1.f - t);
		case 1:
			return bezier_front(t);
		case 2:
			return bezier_left(t);
		case 3:
			return bezier_right(t);
		default:
			return vec3(0, 0, 0);
	}
}

mat4 Translate(float translateX, float translateY, float translateZ)
{
	return transpose(
		mat4(
			1, 0, 0, translateX,
			0, 1, 0, translateY,
			0, 0, 1, translateZ,
			0, 0, 0, 1
		)
	);
}

mat4 Scale(float scaleX, float scaleY, float scaleZ)
{
	return transpose(
		mat4(
			scaleX, 0, 0, 0,
			0, scaleY, 0, 0,
			0, 0, scaleZ, 0,
			0, 0, 0, 1
		)
	);
}

mat4 RotateOY(float radians)
{
	return transpose(
		mat4(
			cos(radians), 0, sin(radians), 0,
			0, 1, 0, 0,
			-sin(radians), 0, cos(radians), 0,
			0, 0, 0, 1
		)
	);
}



float isAtPoint(float curve_step, float eps) {				// if it is, return the point index, else return -1
	float inf_point = floor(main_progress / curve_step);
	float sup_point = inf_point + 1;
	if (main_progress - inf_point * curve_step < eps)
		return inf_point;
	if (abs(main_progress - sup_point * curve_step) < eps)
		return sup_point;
	return -1;
}

float calculateAngle(float prev_point, float next_point, float curve_step) {
	float delta_x = bezier(next_point * curve_step).x - bezier(prev_point * curve_step).x;
	float delta_z = bezier(next_point * curve_step).z - bezier(prev_point * curve_step).z;
	return orientation == 1
			?	(path == 0 
					?	-atan(delta_z / delta_x) + radians(180.f)
					:	-atan(delta_z / delta_x))
			:	(path == 0
					?	-atan(delta_z / delta_x)
					:	-atan(delta_z / delta_x) + radians(180.f));
}

void main() {
	frag_normal = v_normal;
	float curve_step = 1.f / no_gen;
	left_control_p1 = main_control_p4;
	right_control_p1 = main_control_p4;

	mat4 new_Model = mat4(1);
	vec3 new_position;

	float atPoint = isAtPoint(curve_step, 0.01f);
	
	if (atPoint == -1) {
		float inf_point = floor(main_progress / curve_step);
		float sup_point = inf_point + 1;
		float step_completion = main_progress / curve_step - inf_point;
		float delta_x_step = bezier(sup_point * curve_step).x 
							- bezier(inf_point * curve_step).x;
		float delta_z_step = bezier(sup_point * curve_step).z 
							- bezier(inf_point * curve_step).z;
		new_position = vec3(bezier(inf_point * curve_step).x + step_completion * delta_x_step,
							0.f,
							bezier(inf_point * curve_step).z + step_completion * delta_z_step);
		new_Model *= Translate(new_position.x, new_position.y, new_position.z);
		new_Model *= RotateOY(calculateAngle(inf_point, sup_point, curve_step));
	} else {
		new_position = bezier(atPoint * curve_step);
		new_Model *= Translate(new_position.x, new_position.y, new_position.z);
		if (atPoint == 0)
			new_Model *= RotateOY(calculateAngle(0, 1, curve_step));
		else if (atPoint == no_gen)
			new_Model *= RotateOY(calculateAngle(no_gen - 1, no_gen, curve_step));
		else
			new_Model *= RotateOY(calculateAngle(atPoint - 1, atPoint + 1, curve_step));
	}

	new_Model *= Scale(0.2f, 0.2f, 0.2f);
	boatModel = new_Model;






	
	vec3 pos = data[gl_VertexID].pos.xyz;

	float dt = 0.1f;

	pos = vec3(	pos.x + dt * data[gl_VertexID].speed.x, 
				pos.y + dt * data[gl_VertexID].speed.y, 
				pos.z + dt * data[gl_VertexID].speed.z);

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
	




	
	bubbleGlobalCenter = vec4(pos, 1);

	gl_Position = new_Model * vec4(v_position, 1);
}