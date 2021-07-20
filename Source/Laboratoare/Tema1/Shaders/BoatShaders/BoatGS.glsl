#version 430

layout(triangles) in;
layout(triangle_strip, max_vertices = 256) out;

uniform mat4 View;
uniform mat4 Projection;
uniform vec3 eye_position;

layout(location = 0) in vec3 fragNormal[];
layout(location = 1) in float rotHoriz[];
layout(location = 2) in float rotVert[];
layout(location = 3) in float size[];
layout(location = 4) in vec4 bubbleGlobalCenter[];
layout(location = 5) in mat4 boatModel[];

layout(location = 0) out vec3 frag_normal;
layout(location = 1) out vec2 texture_coord;
layout(location = 2) out vec2 is_particle;

//vec3 bubbleCenter = gl_in[0].gl_Position.xyz;
vec3 bubbleCenter = bubbleGlobalCenter[0].xyz;
vec3 forward = normalize(eye_position - bubbleCenter);
vec3 right = normalize(cross(forward, vec3(0, 1, 0)));
vec3 up = normalize(cross(forward, right));

mat4 Translate(vec3 amount) {
	return transpose(
		mat4(
			1,			0,			0,			0,
			0,			1,			0,			0,
			0,			0,			1,			0,
			amount.x,	amount.y,	amount.z,	1
		)
	);
}

mat4 RotateOX(float amount) {
	return transpose(
		mat4(
			1,	0,				0,				0,
			0,	cos(amount),	-sin(amount),	0,
			0,	sin(amount),	cos(amount),	0,
			0,	0,				0,				1
		)
	);
}

mat4 RotateOY(float amount) {
	return transpose(
		mat4(
			cos(amount),	0,	-sin(amount),	0,
			0,				1,	0,				0,
			sin(amount),	0,	cos(amount),	0,
			0,				0,	0,				1
		)
	);
}

mat4 RotateOZ(float amount) {
	return transpose(
		mat4(
			cos(amount),	-sin(amount),	0,	0,
			sin(amount),	cos(amount),	0,	0,
			0,				0,				1,	0,
			0,				0,				0,	1
		)
	);
}

void EmitPoint(vec2 offset, vec3 sampleCenter3D) {
	vec3 pos_eye_perspective = offset.x * right * 3 + offset.y * up * 3 + sampleCenter3D;
	gl_Position = Projection * View * vec4(pos_eye_perspective, 1);
	EmitVertex();
}

void RenderSample(float sampleSize, vec3 sampleCenter3D) {
	texture_coord = vec2(0, 0);
	EmitPoint(vec2(-sampleSize, -sampleSize), sampleCenter3D);

	texture_coord = vec2(1, 0);
	EmitPoint(vec2(sampleSize, -sampleSize), sampleCenter3D);

	texture_coord = vec2(0, 1);
	EmitPoint(vec2(-sampleSize, sampleSize), sampleCenter3D);

	texture_coord = vec2(1, 1);
	EmitPoint(vec2(sampleSize, sampleSize), sampleCenter3D);
}

float magnitude(vec2 v1, vec2 v2) {
	return sqrt(pow(v2.y - v1.y, 2) + pow(v2.x - v1.x, 2));
}

float magnitude(vec3 v1, vec3 v2) {
	return sqrt(pow(v2.z - v1.z, 2) + pow(v2.y - v1.y, 2) + pow(v2.x - v1.x, 2));
}

vec4 SampleCenter3D(vec3 center, int nrSamples, int i, int j) {
	vec2 dII;
	float distanceFromSampleCenter;
	float dBS = (size[0] * 2.f) / nrSamples;
	vec3 sampleCenter;

	dII = -vec2(
			(nrSamples - 1) / 2 - i, 
			(nrSamples - 1) / 2 - j
		);

	sampleCenter = vec3(
						dII.x * dBS + bubbleCenter.x, 
						bubbleCenter.y,
						dII.y * dBS + bubbleCenter.z
					);

	distanceFromSampleCenter = magnitude(sampleCenter, bubbleCenter);

	//vec3 sampleCenterBoatRef = (boatModel[0] * vec4(sampleCenter, 1)).xyz;
	vec3 sampleCenter3D = (Translate(sampleCenter - bubbleCenter)
							* RotateOZ(rotVert[0])
							* RotateOY(rotHoriz[0])
							* Translate(bubbleCenter - sampleCenter)
							* vec4(sampleCenter, 1)
							).xyz;

	return vec4(sampleCenter3D, distanceFromSampleCenter);
}

void main() {
	int nrSamples = 7;
	float sampleSize = 0.01f;

	is_particle = vec2(0, 0);

	gl_Position = Projection * View * gl_in[0].gl_Position;
	frag_normal = fragNormal[0];
	EmitVertex();
	
	gl_Position = Projection * View * gl_in[1].gl_Position;
	frag_normal = fragNormal[1];
	EmitVertex();
	
	gl_Position = Projection * View * gl_in[2].gl_Position;
	frag_normal = fragNormal[2];
	EmitVertex();

	EndPrimitive();

	is_particle = vec2(1, 0);
	for (int i = 0; i < nrSamples; i++) {
		for (int j = 0; j < nrSamples; j++) {
			vec4 sampleCenter3D = SampleCenter3D(bubbleCenter, nrSamples, i, j);
			float distanceFromSampleCenter = sampleCenter3D.w;
			if (distanceFromSampleCenter <= size[0]) {
				vec4 sampleCenterBoatCoords = boatModel[0] * vec4(sampleCenter3D.xyz, 1);
				RenderSample(sampleSize, sampleCenterBoatCoords.xyz);
				EndPrimitive();
			}
		}
	}
	

	
	//RenderSample(0.3f, bubbleCenter);
	//EndPrimitive();
	
	//RenderSample(0.3f, bubbleGlobalCenter[0].xyz + vec3(0, 0, 0));
	//EndPrimitive();
	
}


//
// && asin(0.5f) == radians(30.f) && cos(radians(60.f)) == 0.5f
//