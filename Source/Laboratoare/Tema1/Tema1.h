#pragma once

#include <Component/SimpleScene.h>

class Tema1 : public SimpleScene
{
public:
	Tema1();
	~Tema1();

	void Init() override;

private:
	void FrameStart() override;
	void Update(float deltaTimeSeconds) override;
	void FrameEnd() override;

	int RandomInt();
	float RandomFloat(float lim_inf, float lim_sup, int precision);

	unsigned int UploadCubeMapTexture(const std::string& posx, const std::string& posy, const std::string& posz, const std::string& negx, const std::string& negy, const std::string& negz);
	void RenderMeshInstanced(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, int instances, const glm::vec3& color = glm::vec3(1), std::string texName = "nothing");

	void DispatchMainControlPoints(Shader* shader);
	void DispatchFrontControlPoints(Shader* shader);
	void DispatchRightControlPoints(Shader* shader);
	void DispatchLeftControlPoints(Shader* shader);
	void DispatchSamplingParams(Shader* shader);
	void DispatchCameraPosition(Shader* shader);

	void ApplyMainRiverShader();
	void ApplyFrontRiverShader();
	void ApplyRightRiverShader();
	void ApplyLeftRiverShader();

	glm::vec3 bezier_main(float t);
	float GetCorrectElapsedTime();
	void ResetElapsedTime(float value);
	void UpdatePathCompletion(float deltaTime, float boatSpeed);
	void ChoosePath();

	void ApplyMountainEdgeLeftShader();
	void ApplyMountainEdgeRightShader();
	void ApplyMountainCenterLeftShader();
	void ApplyMountainCenterRightShader();

	void OnInputUpdate(float deltaTime, int mods) override;
	void OnKeyPress(int key, int mods) override;
	void OnKeyRelease(int key, int mods) override;
	void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
	void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
	void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
	void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
	void OnWindowResize(int width, int height) override;


	protected:
		int seed;

		glm::vec3 main_control_p1, main_control_p2, main_control_p3, main_control_p4;
		glm::vec3 front_control_p1, front_control_p2, front_control_p3, front_control_p4;
		glm::vec3 right_control_p1, right_control_p2, right_control_p3, right_control_p4;
		glm::vec3 left_control_p1, left_control_p2, left_control_p3, left_control_p4;

		unsigned int no_of_generated_points, no_of_instances;
		float branch_width, mountain_width, mountain_height;
		float max_translate;
		int cubeMapTextureID, cubeMapTextureID2;
		float firstFrameElapsedTime;
		int path; float pathCompletion; int orientation;	// -1 for descendent, 1 for ascendent
		enum pathss {MAIN, FRONT, LEFT, RIGHT};

		int numberOfBubbles;

	private:
		std::unordered_map<std::string, Texture2D*> mapTextures;
};
