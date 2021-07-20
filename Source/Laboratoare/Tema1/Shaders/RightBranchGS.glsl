#version 430
layout(lines) in;
layout(triangle_strip, max_vertices = 256) out;

uniform mat4 View;
uniform mat4 Projection;
uniform vec3 main_control_p1, main_control_p2, main_control_p3, main_control_p4;
uniform vec3 front_control_p1, front_control_p2, front_control_p3, front_control_p4;
uniform vec3 right_control_p2, right_control_p3, right_control_p4;
uniform int no_inst;
uniform int no_gen;
uniform float branch_width;
uniform float elapsedTime;

layout(location = 0) in int instance[2];
layout(location = 0) out vec2 texCoord;
layout(location = 1) out vec3 point_position;	// in world coords
layout(location = 2) out vec3 point_normal;		// in world coords

vec3 right_control_p1;
vec2 low_x_point;
vec2 high_x_point;

vec2 equidistant_points[2];	// primul vec2 are x negativ (situat in STANGA cursului raului), al doilea e varianta cu x pozitiv (situat in DREAPTA cursului raului) // EDIT: nu mai conteaza ce mai zis mai devreme, pentru ca de la ultimul update se alege punctul cu y maxim sau cu y minim in functie de cerinte, deci cele 2 elemente pot fi asezate in orice ordine aici

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

vec3 bezier_right(float t) {
	return right_control_p1 * pow((1 - t), 3) 
			+ right_control_p2 * 3 * t * pow((1 - t), 2) 
			+ right_control_p3 * 3 * pow(t, 2) * (1 - t) 
			+ right_control_p4 * pow(t, 3);
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

vec3 whichBezier(int i, float curve_step) {
	return i <= 0
		? bezier_main((no_gen + i) * curve_step)
		: bezier_front(i * curve_step);
}

vec2 point(int instance, int i, float curve_step, float width) {
	if (i == 0) {
		equidistant_points[0] = low_x_point;
		equidistant_points[1] = high_x_point;
	} else if (i == no_gen) {
		calculateEdgePoints(vec3To2(bezier_right((i - 1) * curve_step)), 
							vec2(0, 0), 
							vec3To2(bezier_right(i * curve_step)), 
							width, 
							false, 
							true);
	} else {
		calculateEdgePoints(vec3To2(bezier_right((i - 1) * curve_step)), 
							vec3To2(bezier_right(i * curve_step)), 
							vec3To2(bezier_right((i + 1) * curve_step)), 
							width, 
							false, 
							false);
	}

	if (instance == -1) {						// adica marginea stanga (negativa dpdv al coordonatei x)
		return minX(equidistant_points[0], equidistant_points[1]);
	} else if (instance == 1) {					// adica marginea dreapta (pozitiva dpdv al coordonatei x)
		return maxX(equidistant_points[0], equidistant_points[1]);
	} else {									// punctele din mijlocul raului (dpdv al latimii) sunt generate exact de curba bezier_front
		return vec3To2(bezier_right(i * curve_step));
	}
}

void calculatePositionAndNormal(vec2 pos_2D, float wave_intensity, float wave_frequency) {
	float heightOffset;
	float wave_variable;

	wave_variable = pos_2D.x + elapsedTime * wave_frequency;
	heightOffset = sin(wave_variable) * wave_intensity + wave_intensity;
	point_position = vec2To3(pos_2D, heightOffset);
	point_normal = normalize(vec3(-cos(wave_variable) * wave_intensity, 1, 0));
	gl_Position = Projection * View * vec4(point_position, 1.f);
}

void makeWaterPrimitive(int instance, int i, bool inferior, float curve_step, float width, float wave_intensity, float wave_frequency) {
	vec2 point_2D_pos;
	
	if (inferior) {
		texCoord = vec2(0, 0);
		point_2D_pos = point(instance - 1, i, curve_step, width);
		calculatePositionAndNormal(point_2D_pos, wave_intensity, wave_frequency);		EmitVertex();

		texCoord = vec2(0, 1);
		point_2D_pos = point(instance, i, curve_step, width);
		calculatePositionAndNormal(point_2D_pos, wave_intensity, wave_frequency);		EmitVertex();

		texCoord = vec2(1, 0);
		point_2D_pos = point(instance - 1, i + 1, curve_step, width);
		calculatePositionAndNormal(point_2D_pos, wave_intensity, wave_frequency);		EmitVertex();

		EndPrimitive();
	} else {
		texCoord = vec2(1, 0);
		point_2D_pos = point(instance - 1, i + 1, curve_step, width);
		calculatePositionAndNormal(point_2D_pos, wave_intensity, wave_frequency);		EmitVertex();

		texCoord = vec2(1, 1);
		point_2D_pos = point(instance, i + 1, curve_step, width);
		calculatePositionAndNormal(point_2D_pos, wave_intensity, wave_frequency);		EmitVertex();

		texCoord = vec2(0, 1);
		point_2D_pos = point(instance, i, curve_step, width);
		calculatePositionAndNormal(point_2D_pos, wave_intensity, wave_frequency);		EmitVertex();

		EndPrimitive();
	}
}

void main()
{
	if (instance[0] < no_inst)
	{
		float curve_step = 1.f / no_gen;

		calculateEdgePoints(vec3To2(bezier_main((no_gen - 3) * curve_step)), 
							vec3To2(bezier_main((no_gen - 2) * curve_step)), 
							vec3To2(bezier_main((no_gen - 1) * curve_step)), 
							1.f, 
							false, 
							false);	// pentru punctul cu x minim (de la care porneste branch-ul drept)
		low_x_point = equidistant_points[1];	// doar cel "din dreapta" ne intereseaza in urma apelului de mai sus
		calculateEdgePoints(vec3To2(bezier_front(0)), 
							vec3To2(bezier_front(1 * curve_step)), 
							vec3To2(bezier_front(2 * curve_step)), 
							branch_width, 
							false, 
							false);
		high_x_point = equidistant_points[1];

		right_control_p1 = vec3((low_x_point.x + high_x_point.x) / 2.f, 
								1.f, 
								(low_x_point.y + high_x_point.y) / 2.f);
		
		for (int i = 0; i < no_gen; i++) {
			makeWaterPrimitive(instance[0], i, true, curve_step, branch_width, 0.02f, 5.f);
			makeWaterPrimitive(instance[0], i, false, curve_step, branch_width, 0.02f, 5.f);
		}
	}
}
