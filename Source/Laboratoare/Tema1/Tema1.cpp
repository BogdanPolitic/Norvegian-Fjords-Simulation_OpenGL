#include "Tema1.h"

#include <vector>
#include <iostream>

#include <stb/stb_image.h>

#include <Core/Engine.h>

using namespace std;

// Order of function calling can be seen in "Source/Core/World.cpp::LoopUpdate()"
// https://github.com/UPB-Graphics/Framework-EGC/blob/master/Source/Core/World.cpp

// JSYK: In aceasta scena, axa OX e rosie (dreapta-stanga), axa OY e verde (sus-jos), axa OZ e albastra (fata-spate). Ce am scris in paranteze sunt directiile&sensurile fata de rotatia initiala a camerei observatorului.

struct Bubble {
	glm::vec4 iniPos;
	glm::vec4 pos;
	glm::vec4 speed;
	glm::vec4 rotation;
	glm::vec4 size;

	Bubble() {};

	Bubble(const glm::vec4& iniPos, const glm::vec4& pos, const glm::vec4& speed, const glm::vec4 rotation, const glm::vec4 size) {
		SetInitial(iniPos, speed, rotation, size);
	}

	void SetInitial(const glm::vec4& iniPos, const glm::vec4& speed, const glm::vec4 rotation, const glm::vec4 size) {
		this->iniPos = iniPos;
		this->pos = iniPos;
		this->speed = speed;
		this->rotation = rotation;
		this->size = size;
	}
};

ParticleEffect<Bubble>* bubbleEffect;

Tema1::Tema1()
{
}

Tema1::~Tema1()
{
}

glm::vec3 Tema1::bezier_main(float t) {
	return main_control_p1 * pow((1 - t), 3)
		+ main_control_p2 * 3.f * t * pow((1 - t), 2)
		+ main_control_p3 * 3.f * pow(t, 2) * (1 - t)
		+ main_control_p4 * pow(t, 3);
}

int Tema1::RandomInt() {
	return seed = ((int) (Engine::GetElapsedTime() * glm::sqrt(5) * (7904553.344f + seed)) % 50066 + 17094.33f) / 1;
}

float Tema1::RandomFloat(float lim_inf, float lim_sup, int precision) {
	return lim_inf + ((float) (RandomInt() % precision) / precision) * (lim_sup - lim_inf);
}

void Tema1::Init()
{
	auto camera = GetSceneCamera();
	camera->SetPositionAndRotation(glm::vec3(0, 8, 0), glm::quat(glm::vec3(-60 * TO_RADIANS, 0, 0)));
	camera->Update();

	ToggleGroundPlane();
	firstFrameElapsedTime = 0.f;
	pathCompletion = 1.f;
	orientation = -1;

	std::string texturePath = RESOURCE_PATH::TEXTURES + "Sky/";
	std::string texturePath2 = RESOURCE_PATH::TEXTURES + "Cube/";

	{
		Mesh* mesh = new Mesh("cube");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "box.obj");
		mesh->UseMaterials(false);
		meshes[mesh->GetMeshID()] = mesh;
	}

	{
		Mesh* mesh = new Mesh("boat");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Models", "boat.obj");
		mesh->UseMaterials(true);
		meshes[mesh->GetMeshID()] = mesh;
	}

	{
		Mesh* mesh = new Mesh("bubble");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Models", "circle.obj");
		meshes[mesh->GetMeshID()] = mesh;
	}

	{
		Shader* shader = new Shader("CubeMap");
		shader->AddShader("Source/Laboratoare/Tema1/Shaders/CubeMapVS.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/Laboratoare/Tema1/Shaders/CubeMapFS.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	{
		Shader* shader = new Shader("MainRiver");
		shader->AddShader("Source/Laboratoare/Tema1/Shaders/WaterVS.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/Laboratoare/Tema1/Shaders/MainRiverGS.glsl", GL_GEOMETRY_SHADER);
		shader->AddShader("Source/Laboratoare/Tema1/Shaders/WaterReflectionFS.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	{
		Shader* shader = new Shader("FrontRiver");
		shader->AddShader("Source/Laboratoare/Tema1/Shaders/WaterVS.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/Laboratoare/Tema1/Shaders/FrontBranchGS.glsl", GL_GEOMETRY_SHADER);
		shader->AddShader("Source/Laboratoare/Tema1/Shaders/WaterReflectionFS.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	{
		Shader* shader = new Shader("LeftRiver");
		shader->AddShader("Source/Laboratoare/Tema1/Shaders/WaterVS.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/Laboratoare/Tema1/Shaders/LeftBranchGS.glsl", GL_GEOMETRY_SHADER);
		shader->AddShader("Source/Laboratoare/Tema1/Shaders/WaterReflectionFS.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	{
		Shader* shader = new Shader("RightRiver");
		shader->AddShader("Source/Laboratoare/Tema1/Shaders/WaterVS.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/Laboratoare/Tema1/Shaders/RightBranchGS.glsl", GL_GEOMETRY_SHADER);
		shader->AddShader("Source/Laboratoare/Tema1/Shaders/WaterReflectionFS.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	{
		Shader* shader = new Shader("MountainEdgeLeft");
		shader->AddShader("Source/Laboratoare/Tema1/Shaders/WaterVS.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/Laboratoare/Tema1/Shaders/MountainEdgeLeft.glsl", GL_GEOMETRY_SHADER);
		shader->AddShader("Source/Laboratoare/Tema1/Shaders/WaterFS.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	{
		Shader* shader = new Shader("MountainEdgeRight");
		shader->AddShader("Source/Laboratoare/Tema1/Shaders/WaterVS.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/Laboratoare/Tema1/Shaders/MountainEdgeRight.glsl", GL_GEOMETRY_SHADER);
		shader->AddShader("Source/Laboratoare/Tema1/Shaders/WaterFS.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	{
		Shader* shader = new Shader("MountainCenterLeft");
		shader->AddShader("Source/Laboratoare/Tema1/Shaders/WaterVS.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/Laboratoare/Tema1/Shaders/MountainCenterLeft.glsl", GL_GEOMETRY_SHADER);
		shader->AddShader("Source/Laboratoare/Tema1/Shaders/WaterFS.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	{
		Shader* shader = new Shader("MountainCenterRight");
		shader->AddShader("Source/Laboratoare/Tema1/Shaders/WaterVS.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/Laboratoare/Tema1/Shaders/MountainCenterRight.glsl", GL_GEOMETRY_SHADER);
		shader->AddShader("Source/Laboratoare/Tema1/Shaders/WaterFS.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	{
		Shader* shader = new Shader("Boat");
		shader->AddShader("Source/Laboratoare/Tema1/Shaders/BoatShaders/BoatVS.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/Laboratoare/Tema1/Shaders/BoatShaders/BoatGS.glsl", GL_GEOMETRY_SHADER);
		shader->AddShader("Source/Laboratoare/Tema1/Shaders/BoatShaders/BoatFS.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	{
		Shader* shader = new Shader("Bubble");
		shader->AddShader("Source/Laboratoare/Tema1/Shaders/BubbleShaders/BubbleVS.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/Laboratoare/Tema1/Shaders/BubbleShaders/BubbleGS.glsl", GL_GEOMETRY_SHADER);
		shader->AddShader("Source/Laboratoare/Tema1/Shaders/BubbleShaders/BubbleFS.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}
	
	{
		Texture2D* texture = new Texture2D();
		texture->Load2D("Resources/Textures/water.png", GL_MIRRORED_REPEAT);
		mapTextures["water"] = texture;
	}

	{
		Texture2D* texture = new Texture2D();
		texture->Load2D("Resources/Textures/mountain.png", GL_MIRRORED_REPEAT);
		mapTextures["mountain"] = texture;
	}

	{
		Texture2D* texture = new Texture2D();
		texture->Load2D("Resources/Textures/default.png", GL_MIRRORED_REPEAT);
		mapTextures["default"] = texture;
	}

	{
		Texture2D* texture = new Texture2D();
		texture->Load2D("Resources/Textures/so_simple.png", GL_MIRRORED_REPEAT);
		mapTextures["so_simple"] = texture;
	}

	cubeMapTextureID = UploadCubeMapTexture(
		texturePath + "posx_.bmp",
		texturePath + "posy_.bmp",
		texturePath + "posz_.bmp",
		texturePath + "negx_.bmp",
		texturePath + "negy_.bmp",
		texturePath + "negz_.bmp"
	);

	//parameters related to surface generation
	no_of_generated_points = 15;	//number of points on a Bezier curve
	no_of_instances = 2;			//number of instances (number of curves that contain the surface)
	max_translate = 12.0f;			//for the translation surface, it's the distance between the first and the last curve
	branch_width = 0.7f;
	mountain_width = 1.5f;
	mountain_height = 2.5f;

	//define control points
	main_control_p1 = glm::vec3(-2.5, 0.f, -4.0);
	main_control_p2 = glm::vec3(1.5, 0.f, -2.5);
	main_control_p3 = glm::vec3(3.0, 0.f, -1.5);
	main_control_p4 = glm::vec3(6.0, 0.f, -4.0);

	front_control_p1 = glm::vec3(6.0, 0.f, -4.0);
	front_control_p2 = glm::vec3(9.5, 0.f, -6.5);
	front_control_p3 = glm::vec3(11.0, 0.f, -7.0);
	front_control_p4 = glm::vec3(13.5, 0.f, -6.0);

	left_control_p1 = glm::vec3();
	left_control_p2 = glm::vec3(6.0, 0.f, -9.0);
	left_control_p3 = glm::vec3(8.0, 0.f, -9.5);
	left_control_p4 = glm::vec3(11.0, 0.f, -11.0);

	right_control_p1 = glm::vec3();
	right_control_p2 = glm::vec3(8.0, 0.f, -1.5);
	right_control_p3 = glm::vec3(9.5, 0.f, -0.5);
	right_control_p4 = glm::vec3(12.5, 0.f, 0.5);


	// Create a bogus mesh with 2 points (a line)
	{
		vector<VertexFormat> vertices
		{
			VertexFormat(glm::vec3(-4.0, -2.5,  1.0), glm::vec3(0, 1, 0)),
			VertexFormat(glm::vec3(-4.0, 5.5,  1.0), glm::vec3(0, 1, 0))
		};

		vector<unsigned short> indices =
		{
			0, 1
		};

		meshes["surface"] = new Mesh("generated initial surface points");
		meshes["surface"]->InitFromData(vertices, indices);
		meshes["surface"]->SetDrawMode(GL_LINES);
	}

	// Create the SSBO for the bubble effect:
	{
		numberOfBubbles = meshes["boat"]->positions.size();
		cout << "cate varfuri? " << meshes["boat"]->positions.size() << endl;

		bubbleEffect = new ParticleEffect<Bubble>();
		bubbleEffect->Generate(numberOfBubbles, true);

		auto bubbleSSBO = bubbleEffect->GetParticleBuffer();
		Bubble* data = const_cast<Bubble*>(bubbleSSBO->GetBuffer());
		for (int i = 0; i < numberOfBubbles; i++) {
			data[i].SetInitial(	glm::vec4(	RandomFloat(-5.f, 5.f, 1000),
											RandomFloat(-5.f, 5.f, 1000),
											RandomFloat(-10.f, 10.f, 1000),
											0) * 0.1f, 
								glm::vec4(0.5f, -0.5f, -0.7f, 0) * 1.0f, 
								glm::vec4(1 + i, 3 - i * 2, -2 + i * -2, 0), 
								glm::vec4(0.3f, 0, 0, 0));
		}

		bubbleSSBO->SetBufferData(data);
	}
}

unsigned int Tema1::UploadCubeMapTexture(const std::string& posx, const std::string& posy, const std::string& posz, const std::string& negx, const std::string& negy, const std::string& negz)
{
	int width, height, chn;

	unsigned char* data_posx = stbi_load(posx.c_str(), &width, &height, &chn, 0);
	unsigned char* data_posy = stbi_load(posy.c_str(), &width, &height, &chn, 0);
	unsigned char* data_posz = stbi_load(posz.c_str(), &width, &height, &chn, 0);
	unsigned char* data_negx = stbi_load(negx.c_str(), &width, &height, &chn, 0);
	unsigned char* data_negy = stbi_load(negy.c_str(), &width, &height, &chn, 0);
	unsigned char* data_negz = stbi_load(negz.c_str(), &width, &height, &chn, 0);

	// TODO - create OpenGL texture
	unsigned int textureID = 0;
	glGenTextures(1, &textureID);

	// TODO - bind the texture
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	float maxAnisotropy;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// TODO - load texture information for each face
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_posx);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_posy);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_posz);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_negx);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_negy);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_negz);

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// free memory
	SAFE_FREE(data_posx);
	SAFE_FREE(data_posy);
	SAFE_FREE(data_posz);
	SAFE_FREE(data_negx);
	SAFE_FREE(data_negy);
	SAFE_FREE(data_negz);

	return textureID;
}


void Tema1::FrameStart()
{
	// clears the color buffer (using the previously set color) and depth buffer
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::ivec2 resolution = window->GetResolution();
	// sets the screen area where to draw
	glViewport(0, 0, resolution.x, resolution.y);
}

void Tema1::RenderMeshInstanced(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, int instances, const glm::vec3& color, std::string texName)
{
	if (!mesh || !shader || !shader->GetProgramID())
		return;

	glUseProgram(shader->program);

	GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
	glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

	glm::mat4 viewMatrix = GetSceneCamera()->GetViewMatrix();
	int loc_view_matrix = glGetUniformLocation(shader->program, "View");
	glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	glm::mat4 projectionMatrix = GetSceneCamera()->GetProjectionMatrix();
	int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
	glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	int loc_color = glGetUniformLocation(shader->program, "_color");
	glUniform3f(loc_color, color.x, color.y, color.z);

	glUniform1i(glGetUniformLocation(shader->program, "waterTexture"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mapTextures[texName]->GetTextureID());

	glBindVertexArray(mesh->GetBuffers()->VAO);
	glDrawElementsInstanced(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, (void*)0, instances);
}

void Tema1::DispatchMainControlPoints(Shader* shader) {
	glUniform3f(glGetUniformLocation(shader->program, "main_control_p1"), main_control_p1.x, main_control_p1.y, main_control_p1.z);
	glUniform3f(glGetUniformLocation(shader->program, "main_control_p2"), main_control_p2.x, main_control_p2.y, main_control_p2.z);
	glUniform3f(glGetUniformLocation(shader->program, "main_control_p3"), main_control_p3.x, main_control_p3.y, main_control_p3.z);
	glUniform3f(glGetUniformLocation(shader->program, "main_control_p4"), main_control_p4.x, main_control_p4.y, main_control_p4.z);
}

void Tema1::DispatchFrontControlPoints(Shader* shader) {
	glUniform3f(glGetUniformLocation(shader->program, "front_control_p1"), front_control_p1.x, front_control_p1.y, front_control_p1.z);
	glUniform3f(glGetUniformLocation(shader->program, "front_control_p2"), front_control_p2.x, front_control_p2.y, front_control_p2.z);
	glUniform3f(glGetUniformLocation(shader->program, "front_control_p3"), front_control_p3.x, front_control_p3.y, front_control_p3.z);
	glUniform3f(glGetUniformLocation(shader->program, "front_control_p4"), front_control_p4.x, front_control_p4.y, front_control_p4.z);
}

void Tema1::DispatchLeftControlPoints(Shader* shader) {
	glUniform3f(glGetUniformLocation(shader->program, "left_control_p2"), left_control_p2.x, left_control_p2.y, left_control_p2.z);
	glUniform3f(glGetUniformLocation(shader->program, "left_control_p3"), left_control_p3.x, left_control_p3.y, left_control_p3.z);
	glUniform3f(glGetUniformLocation(shader->program, "left_control_p4"), left_control_p4.x, left_control_p4.y, left_control_p4.z);
}

void Tema1::DispatchRightControlPoints(Shader* shader) {
	glUniform3f(glGetUniformLocation(shader->program, "right_control_p2"), right_control_p2.x, right_control_p2.y, right_control_p2.z);
	glUniform3f(glGetUniformLocation(shader->program, "right_control_p3"), right_control_p3.x, right_control_p3.y, right_control_p3.z);
	glUniform3f(glGetUniformLocation(shader->program, "right_control_p4"), right_control_p4.x, right_control_p4.y, right_control_p4.z);
}

void Tema1::DispatchSamplingParams(Shader* shader) {
	glUniform1i(glGetUniformLocation(shader->program, "no_inst"), no_of_instances);
	int loc_no_of_generated_points = glGetUniformLocation(shader->program, "no_gen");
	glUniform1i(loc_no_of_generated_points, no_of_generated_points);
	int loc_branch_width = glGetUniformLocation(shader->program, "branch_width");
	glUniform1f(loc_branch_width, branch_width);
}

void Tema1::DispatchCameraPosition(Shader* shader) {
	glm::vec3 cam_pos = GetSceneCamera()->transform->GetWorldPosition();
	glUniform3f(glGetUniformLocation(shader->program, "camera_position"), cam_pos.x, cam_pos.y, cam_pos.z);
}

void Tema1::ApplyMainRiverShader() {
	Shader* shader = shaders["MainRiver"];
	shader->Use();

	DispatchMainControlPoints(shader);
	DispatchSamplingParams(shader);

	DispatchCameraPosition(shader);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);
	int loc_texture = shader->GetUniformLocation("skyboxTexture");
	glUniform1i(loc_texture, 0);

	glUniform1f(glGetUniformLocation(shader->program, "elapsedTime"), Engine::GetElapsedTime());
}

void Tema1::ApplyFrontRiverShader() {
	Shader* shader = shaders["FrontRiver"];
	shader->Use();

	DispatchMainControlPoints(shader);
	DispatchFrontControlPoints(shader);
	DispatchSamplingParams(shader);

	DispatchCameraPosition(shader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);
	int loc_texture = shader->GetUniformLocation("skyboxTexture");
	glUniform1i(loc_texture, 0);

	glUniform1f(glGetUniformLocation(shader->program, "elapsedTime"), Engine::GetElapsedTime());
}

void Tema1::ApplyLeftRiverShader() {
	Shader* shader = shaders["LeftRiver"];
	shader->Use();

	DispatchMainControlPoints(shader);
	DispatchFrontControlPoints(shader);
	DispatchLeftControlPoints(shader);
	DispatchSamplingParams(shader);

	DispatchCameraPosition(shader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);
	int loc_texture = shader->GetUniformLocation("skyboxTexture");
	glUniform1i(loc_texture, 0);

	glUniform1f(glGetUniformLocation(shader->program, "elapsedTime"), Engine::GetElapsedTime());
}

void Tema1::ApplyRightRiverShader() {
	Shader* shader = shaders["RightRiver"];
	shader->Use();

	DispatchMainControlPoints(shader);
	DispatchFrontControlPoints(shader);
	DispatchRightControlPoints(shader);
	DispatchSamplingParams(shader);

	DispatchCameraPosition(shader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);
	int loc_texture = shader->GetUniformLocation("skyboxTexture");
	glUniform1i(loc_texture, 0);

	glUniform1f(glGetUniformLocation(shader->program, "elapsedTime"), Engine::GetElapsedTime());
}

void Tema1::ApplyMountainEdgeLeftShader() {
	Shader* shader = shaders["MountainEdgeLeft"];
	shader->Use();

	DispatchMainControlPoints(shader);
	DispatchFrontControlPoints(shader);
	DispatchLeftControlPoints(shader);
	DispatchSamplingParams(shader);

	glUniform1f(glGetUniformLocation(shader->program, "mountain_width"), mountain_width);
	glUniform1f(glGetUniformLocation(shader->program, "mountain_height"), mountain_height);
}

void Tema1::ApplyMountainEdgeRightShader() {
	Shader* shader = shaders["MountainEdgeRight"];
	shader->Use();

	DispatchMainControlPoints(shader);
	DispatchFrontControlPoints(shader);
	DispatchRightControlPoints(shader);
	DispatchSamplingParams(shader);

	glUniform1f(glGetUniformLocation(shader->program, "mountain_width"), mountain_width);
	glUniform1f(glGetUniformLocation(shader->program, "mountain_height"), mountain_height);
}

void Tema1::ApplyMountainCenterLeftShader() {
	Shader* shader = shaders["MountainCenterLeft"];
	shader->Use();

	DispatchMainControlPoints(shader);
	DispatchFrontControlPoints(shader);
	DispatchLeftControlPoints(shader);
	DispatchSamplingParams(shader);

	glUniform1f(glGetUniformLocation(shader->program, "mountain_width"), mountain_width);
	glUniform1f(glGetUniformLocation(shader->program, "mountain_height"), mountain_height);
}

void Tema1::ApplyMountainCenterRightShader() {
	Shader* shader = shaders["MountainCenterRight"];
	shader->Use();

	DispatchMainControlPoints(shader);
	DispatchFrontControlPoints(shader);
	DispatchRightControlPoints(shader);
	DispatchSamplingParams(shader);

	glUniform1f(glGetUniformLocation(shader->program, "mountain_width"), mountain_width);
	glUniform1f(glGetUniformLocation(shader->program, "mountain_height"), mountain_height);
}

float Tema1::GetCorrectElapsedTime() {
	return Engine::GetElapsedTime() - firstFrameElapsedTime;
}

void Tema1::ResetElapsedTime(float offset) {
	firstFrameElapsedTime = Engine::GetElapsedTime() - offset;
}

void Tema1::ChoosePath() {
	int randomPath = rand() % 4;
	switch (randomPath) {
	case 0:
		path = MAIN;
		break;
	case 1:
		path = FRONT;
		break;
	case 2:
		path = LEFT;
		break;
	case 3:
		path = RIGHT;
		break;
	default:
		path = -1;
	}
}

void Tema1::UpdatePathCompletion(float deltaTime, float boatSpeed) {
	if (orientation == 1)
		pathCompletion += deltaTime * boatSpeed;
	else
		pathCompletion -= deltaTime * boatSpeed;

	if (pathCompletion > 1.f) {	// reached a dead end; switch orientation to come back:
		pathCompletion = 2 - pathCompletion;
		orientation = -1;
	}
	else if (pathCompletion < 0.f) {	// reached the intersection; choose a branch to switch to (also switch orientation):
		pathCompletion = -pathCompletion;
		orientation = 1;
		ChoosePath();
	}
}

void Tema1::Update(float deltaTimeSeconds)
{
	ClearScreen();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// era GL_LINE inainte (pentru wireframe!)

	if (firstFrameElapsedTime == 0.f)
		firstFrameElapsedTime = Engine::GetElapsedTime();

	auto camera = GetSceneCamera();

	{
		Shader* shader = shaders["CubeMap"];
		shader->Use();

		glm::mat4 modelMatrix = glm::scale(glm::mat4(1), glm::vec3(30));

		glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
		glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);
		int loc_texture = shader->GetUniformLocation("texture_cubemap");
		glUniform1i(loc_texture, 0);

		meshes["cube"]->Render();
	}

	
	Mesh* mesh = meshes["surface"];

	ApplyMainRiverShader();
	RenderMeshInstanced(mesh, shaders["MainRiver"], glm::mat4(1), no_of_instances, glm::vec3(0, 1, 0), "water");

	ApplyFrontRiverShader();
	RenderMeshInstanced(mesh, shaders["FrontRiver"], glm::mat4(1), no_of_instances, glm::vec3(0, 1, 0), "water");

	ApplyLeftRiverShader();
	RenderMeshInstanced(mesh, shaders["LeftRiver"], glm::mat4(1), no_of_instances, glm::vec3(0, 1, 0), "water");

	ApplyRightRiverShader();
	RenderMeshInstanced(mesh, shaders["RightRiver"], glm::mat4(1), no_of_instances, glm::vec3(0, 1, 0), "water");

	ApplyMountainEdgeLeftShader();
	RenderMeshInstanced(mesh, shaders["MountainEdgeLeft"], glm::mat4(1), no_of_instances, glm::vec3(0, 1, 0), "mountain");

	ApplyMountainEdgeRightShader();
	RenderMeshInstanced(mesh, shaders["MountainEdgeRight"], glm::mat4(1), no_of_instances, glm::vec3(0, 1, 0), "mountain");

	ApplyMountainCenterLeftShader();
	RenderMeshInstanced(mesh, shaders["MountainCenterLeft"], glm::mat4(1), no_of_instances, glm::vec3(0, 1, 0), "mountain");

	ApplyMountainCenterRightShader();
	RenderMeshInstanced(mesh, shaders["MountainCenterRight"], glm::mat4(1), no_of_instances, glm::vec3(0, 1, 0), "mountain");
	
	{
		Shader* shader = shaders["Boat"];
		shader->Use();

		glm::mat4 modelMatrix = glm::mat4(1);

		glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
		glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));

		//glUniform1f(glGetUniformLocation(shader->program, "elapsedTime"), Engine::GetElapsedTime());


		DispatchMainControlPoints(shader);
		DispatchFrontControlPoints(shader);
		DispatchLeftControlPoints(shader);
		DispatchRightControlPoints(shader);

		DispatchSamplingParams(shader);

		glUniform1f(glGetUniformLocation(shader->program, "main_progress"), pathCompletion);
		glUniform1i(glGetUniformLocation(shader->program, "path"), path);
		glUniform1i(glGetUniformLocation(shader->program, "orientation"), orientation);

		bubbleEffect->GetParticleBuffer()->BindBuffer(0);
		meshes["boat"]->Render();

		UpdatePathCompletion(deltaTimeSeconds, 1.0f);
	}

	/*
	{
		glDisable(GL_BLEND);

		Shader* shader = shaders["Bubble"];
		if (shader != NULL && shader->GetProgramID()) {
			shader->Use();
			mapTextures["so_simple"]->BindToTextureUnit(GL_TEXTURE0);
			bubbleEffect->Render(GetSceneCamera(), shader, numberOfBubbles);
		}
	}
	*/
}

void Tema1::FrameEnd()
{
	DrawCoordinatSystem();
}

// Read the documentation of the following functions in: "Source/Core/Window/InputController.h" or
// https://github.com/UPB-Graphics/Framework-EGC/blob/master/Source/Core/Window/InputController.h

void Tema1::OnInputUpdate(float deltaTime, int mods)
{
	// treat continuous update based on input
};

void Tema1::OnKeyPress(int key, int mods)
{
	// tratarea configuratiei suprafetei:
	if (key == GLFW_KEY_3) {		// creste numarul de puncte ce aproximeaza fiecare curba
		no_of_generated_points++;
	}
	if (key == GLFW_KEY_4) {		// scade numarul de puncte ce aproximeaza fiecare curba
		no_of_generated_points--;
	}
	if (key == GLFW_KEY_5) {		// creste numarul de instance ce alcatuieste mesha
		no_of_instances++;
	}
	if (key == GLFW_KEY_6) {		// scade numarul de instance ce alcatuieste mesha
		no_of_instances--;
	}
};

void Tema1::OnKeyRelease(int key, int mods)
{
	// add key release event
};

void Tema1::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
	// add mouse move event
};

void Tema1::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button press event
};

void Tema1::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button release event
}

void Tema1::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
	// treat mouse scroll event
}

void Tema1::OnWindowResize(int width, int height)
{
	// treat window resize event
}
