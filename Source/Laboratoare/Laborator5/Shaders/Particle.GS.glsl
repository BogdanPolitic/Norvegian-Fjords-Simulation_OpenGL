#version 430
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform mat4 View;
uniform mat4 Projection;
uniform vec3 eye_position;

layout(location = 0) out vec2 texture_coord;

vec3 vpos = gl_in[0].gl_Position.xyz;
vec3 forward = normalize(eye_position - vpos);
vec3 right = normalize(cross(forward, vec3(0, 1, 0)));
vec3 up = normalize(cross(forward, right));

vec3 p0 = vec3(0.5f, 0.7f, 0.3f);
vec3 p1 = vec3(0.1f, -0.1f, 0.4f);
vec3 p2 = vec3(1.0f, 1.2f, 0.3f);
vec3 p3 = vec3(0.5f, -0.4f, 0.5f);

void EmitPoint(vec2 offset)
{
	vec3 pos = right * offset.x + up * offset.y + vpos;
	gl_Position = Projection * View * vec4(pos, 1.0);
	EmitVertex();
}

void main()
{
	float ds = 0.05 ;

	// Create a quad using triangle strip
	//
	//	3---------2
	//  | \       |
	//  |    \    |
	//  |       \ |
	//  0---------1
	//
	// Triangles: (0, 1, 3), (1, 2, 3)
	EmitPoint(p0.xy);
	EmitPoint(p1.xy);
	EmitPoint(p3.xy);
	EndPrimitive();
	EmitPoint(p3.xy);
	EmitPoint(p1.xy);
	EmitPoint(p2.xy);
	EndPrimitive();
}
