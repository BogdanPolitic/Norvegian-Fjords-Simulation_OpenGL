#version 430
layout(lines) in;
layout(triangle_strip, max_vertices = 256) out;

uniform mat4 View;
uniform mat4 Projection;
uniform vec3 main_control_p1, main_control_p2, main_control_p3, main_control_p4;
uniform vec3 front_control_p1, front_control_p2, front_control_p3, front_control_p4;
uniform vec3 left_control_p2, left_control_p3, left_control_p4;
uniform int no_inst;
uniform int no_gen;
uniform float branch_width;
uniform float mountain_width;
uniform float mountain_height;

layout(location = 0) in int instance[2];
layout(location = 0) out vec2 texCoord;

vec3 left_control_p1;
vec2 low_x_point;
vec2 high_x_point;

vec2 equidistant_points[2];	// primul vec2 are x negativ (situat in STANGA cursului raului), al doilea e varianta cu x pozitiv (situat in DREAPTA cursului raului)
vec2 mountain_main[64];
vec2 mountain_left[64];

vec2 vec3To2(vec3 vect) {
	return vec2(vect.x, vect.z);
}

vec3 vec2To3(vec2 vect, float height) {
	return vec3(vect.x, height, vect.y);
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

float slope(vec2 p1, vec2 p2) {
	return (p2.y - p1.y) / (p2.x - p1.x);
}

void calculateEdgePoints(vec2 point_inf, vec2 point_int, vec2 point_sup, float width, bool first, bool last) {
	vec2 point_ref = first
						? point_inf
						: last
							? point_sup
							: point_int;

	float m_original = first
				? slope(point_ref, point_sup)
				: last
					? slope(point_inf, point_ref)
					: slope(point_inf, point_sup);

	float m = -1.f / m_original;

	float rez_x, rez_y;
	rez_x = point_ref.x - width / sqrt(1 + pow(m, 2));
	rez_y = m * (rez_x - point_ref.x) + point_ref.y;
	equidistant_points[0] = vec2(rez_x, rez_y);	// STANGA CURSULUI RAULUI

	rez_x = point_ref.x + width / sqrt(1 + pow(m, 2));
	rez_y = m * (rez_x - point_ref.x) + point_ref.y;
	equidistant_points[1] = vec2(rez_x, rez_y);	// DREAPTA CURSULUI RAULUI
}

vec2 minX(vec2 p1, vec2 p2) {
	if (p1.x < p2.x) return p1;
	return p2;
}

vec2 maxX(vec2 p1, vec2 p2) {
	if (p1.x > p2.x) return p1;
	return p2;
}

vec2 minZ(vec2 p1, vec2 p2) {
	if (p1.y < p2.y) return p1;
	return p2;
}

vec2 maxZ(vec2 p1, vec2 p2) {
	if (p1.y > p2.y) return p1;
	return p2;
}

vec3 middleOfSegment(vec2 p1, vec2 p2, float height) {
	return vec3((p1.x + p2.x) / 2.f, height, (p1.y + p2.y) / 2.f);
}

vec2 point_main(int instance, int i, float curve_step, float width) {
	if (i == 0) {
		calculateEdgePoints(vec3To2(bezier_main(0)), 
							vec2(0, 0), 
							vec3To2(bezier_main(1 * curve_step)), 
							width, 
							true, 
							false);
	} else if (i == no_gen) {
		calculateEdgePoints(vec3To2(bezier_main((i - 1) * curve_step)), 
							vec2(0, 0), 
							vec3To2(bezier_main(i * curve_step)), 
							width, 
							false, 
							true);
	} else {
		calculateEdgePoints(vec3To2(bezier_main((i - 1) * curve_step)), 
							vec3To2(bezier_main(i * curve_step)), 
							vec3To2(bezier_main((i + 1) * curve_step)), 
							width, 
							false, 
							false);
	}

	if (instance == -1) {						// adica marginea stanga (negativa dpdv al coordonatei x)
		return minZ(equidistant_points[0], equidistant_points[1]);
	} else if (instance == 1) {					// adica marginea dreapta (pozitiva dpdv al coordonatei x)
		return maxZ(equidistant_points[0], equidistant_points[1]);
	} else {									// punctele din mijlocul raului (dpdv al latimii) sunt generate exact de curba bezier_main
		return vec3To2(bezier_main(i * curve_step));
	}
}

vec2 point_left(int instance, int i, float curve_step, float width) {
	if (i == 0) {
		equidistant_points[0] = low_x_point;
		equidistant_points[1] = high_x_point;
	} else if (i == no_gen) {
		calculateEdgePoints(vec3To2(bezier_left((i - 1) * curve_step)), 
							vec2(0, 0), 
							vec3To2(bezier_left(i * curve_step)), 
							width, 
							false, 
							true);
	} else {
		calculateEdgePoints(vec3To2(bezier_left((i - 1) * curve_step)), 
							vec3To2(bezier_left(i * curve_step)), 
							vec3To2(bezier_left((i + 1) * curve_step)), 
							width, 
							false, 
							false);
	}

	if (instance == -1) {						// adica marginea stanga (negativa dpdv al coordonatei x)
		return minX(equidistant_points[0], equidistant_points[1]);
	} else if (instance == 1) {					// adica marginea dreapta (pozitiva dpdv al coordonatei x)
		return maxX(equidistant_points[0], equidistant_points[1]);
	} else {									// punctele din mijlocul raului (dpdv al latimii) sunt generate exact de curba bezier_front
		return vec3To2(bezier_left(i * curve_step));
	}
}

void makeMountainPrimitive(bool is_corner, int instance, int i, bool is_main, bool inferior, float curve_step, float height, float width) {
	if (is_corner) {
		texCoord = vec2(0, 1);
		gl_Position = Projection * View * vec4(vec2To3(mountain_left[0], height + mountain_height), 1.f);						EmitVertex();
		texCoord = vec2(1, 0);
		gl_Position = Projection * View * vec4(vec2To3(point_main(instance - 1, no_gen - 2, curve_step, 1.f), height), 1.f);	EmitVertex();
		texCoord = vec2(1, 1);
		gl_Position = Projection * View * vec4(vec2To3(point_left(instance - 1, 1, curve_step, width), height), 1.f);				EmitVertex();
		EndPrimitive();
		return;
	}

	if (is_main && inferior) {
		texCoord = vec2(0, 0);
		gl_Position = Projection * View * vec4(vec2To3(mountain_main[i], height + mountain_height), 1.f);								EmitVertex();
		texCoord = vec2(0, 1);
		gl_Position = Projection * View * vec4(vec2To3(point_main(instance - 1, i, curve_step, 1.f), height), 1.f);	EmitVertex();
		texCoord = vec2(1, 0);
		gl_Position = Projection * View * vec4(vec2To3(mountain_main[i + 1], height + mountain_height), 1.f);							EmitVertex();

		EndPrimitive();
	} 
	if (is_main && !inferior) {
		texCoord = vec2(1, 0);
		gl_Position = Projection * View * vec4(vec2To3(mountain_main[i + 1], height + mountain_height), 1.f);								EmitVertex();
		texCoord = vec2(1, 1);
		gl_Position = Projection * View * vec4(vec2To3(point_main(instance - 1, i + 1, curve_step, 1.f), height), 1.f);	EmitVertex();
		texCoord = vec2(0, 1);
		gl_Position = Projection * View * vec4(vec2To3(point_main(instance - 1, i, curve_step, 1.f), height), 1.f);		EmitVertex();

		EndPrimitive();
	}
	if (!is_main && inferior) {
		texCoord = vec2(0, 0);
		gl_Position = Projection * View * vec4(vec2To3(mountain_left[i - 1], height + mountain_height), 1.f);								EmitVertex();
		texCoord = vec2(0, 1);
		gl_Position = Projection * View * vec4(vec2To3(point_left(instance - 1, i, curve_step, width), height), 1.f);	EmitVertex();
		texCoord = vec2(1, 0);
		gl_Position = Projection * View * vec4(vec2To3(mountain_left[i], height + mountain_height), 1.f);									EmitVertex();

		EndPrimitive();
	} 
	if (!is_main && !inferior) {
		texCoord = vec2(0, 1);
		gl_Position = Projection * View * vec4(vec2To3(mountain_left[i], height + mountain_height), 1.f);									EmitVertex();
		texCoord = vec2(1, 0);
		gl_Position = Projection * View * vec4(vec2To3(point_left(instance - 1, i, curve_step, width), height), 1.f);	EmitVertex();
		texCoord = vec2(1, 1);
		gl_Position = Projection * View * vec4(vec2To3(point_left(instance - 1, i + 1, curve_step, width), height), 1.f);									EmitVertex();

		EndPrimitive();
	} 
}

void calculateLowAndHighXPoint(float curve_step) {
	calculateEdgePoints(vec3To2(bezier_main((no_gen - 3) * curve_step)), 
						vec3To2(bezier_main((no_gen - 2) * curve_step)), 
						vec3To2(bezier_main((no_gen - 1) * curve_step)), 
						1.f, 
						false, 
						false);	// pentru punctul cu x minim (de la care porneste branch-ul drept)
	low_x_point = equidistant_points[0];	// doar cel "din stanga" ne intereseaza in urma apelului de mai sus
	calculateEdgePoints(vec3To2(bezier_front(0)), 
						vec3To2(bezier_front(1 * curve_step)), 
						vec3To2(bezier_front(2 * curve_step)), 
						branch_width, 
						false, 
						false);
	high_x_point = equidistant_points[0];
}

void main()
{
	if (instance[0] < no_inst)
	{
		float curve_step = 1.f / no_gen;

		calculateLowAndHighXPoint(curve_step);

		left_control_p1 = middleOfSegment(low_x_point, high_x_point, 1.f);

		vec3 common_point = middleOfSegment(point_main(-1, no_gen - 3, curve_step, 1.f + mountain_width),
											point_left(-1, 1, curve_step, branch_width + mountain_width),
											1.f);

		// calculate mountain_main:
		for (int i = 0; i <= no_gen - 3; i++) {
			mountain_main[i] = point_main(-1, i, curve_step, 1.f + mountain_width);
		}
		mountain_main[no_gen - 2] = vec3To2(common_point);

		// calculate mountain_left:
		mountain_left[0] = vec3To2(common_point);
		for (int i = 1; i < no_gen; i++) {
			mountain_left[i] = point_left(-1, i + 1, curve_step, branch_width + mountain_width);
		}
		
		for (int i = 0; i <= no_gen - 3; i++) {
			makeMountainPrimitive(false, 0, i, true, true, curve_step, 0.0f, branch_width);
			makeMountainPrimitive(false, 0, i, true, false, curve_step, 0.0f, branch_width);
		}
		for (int i = 1; i < no_gen; i++) {
			makeMountainPrimitive(false, 0, i, false, true, curve_step, 0.0f, branch_width);
			makeMountainPrimitive(false, 0, i, false, false, curve_step, 0.0f, branch_width);
		}
		makeMountainPrimitive(true, 0, 0, false, false, curve_step, 0.0f, branch_width);
	}
}