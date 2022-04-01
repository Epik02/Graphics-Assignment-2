/*
Week 3 tutorial sample - LERP plus paths
Quinn Daggett 2021
*/

#include "NOU/App.h"
#include "NOU/Input.h"
#include "NOU/Entity.h"
#include "NOU/CCamera.h"
#include "NOU/CMeshRenderer.h"
#include "CPathAnimator.h"
#include "Tools/PathUtility.h"
#include "NOU/Shader.h"
#include "NOU/GLTFLoader.h"
#include <iostream>
#include "imgui.h"

#include <memory>

using namespace nou;

std::unique_ptr<ShaderProgram> prog_texLit, prog_lit, prog_unlit;
std::unique_ptr<Mesh> mesh_ducky, mesh_box;
std::unique_ptr<Texture2D> tex2D_ducky;
std::unique_ptr<Material> mat_ducky, mat_unselected, mat_selected, mat_line;

// This helps keep our main() clean
void LoadDefaultResources();

// Templated LERP function
template<typename T>
T LERP(const T& p0, const T& p1, float t)
{
	return (1.0f - t) * p0 + t * p1;
}

int main()
{
	// Create window and set clear color
	App::Init("Week 3 tutorial - LERP plus paths", 800, 600);
	App::SetClearColor(glm::vec4(0.0f, 0.27f, 0.4f, 1.0f));

	// Initialize ImGui
	App::InitImgui();

	// Load in our model/texture resources
	LoadDefaultResources();

	// Create and set up camera
	Entity ent_camera = Entity::Create();
	CCamera& cam = ent_camera.Add<CCamera>(ent_camera);
	cam.Perspective(60.0f, 1.0f, 0.1f, 100.0f);
	ent_camera.transform.m_pos = glm::vec3(0.0f, 0.0f, 4.0f);

	// Creating duck entity
	Entity ent_ducky = Entity::Create();
	ent_ducky.Add<CMeshRenderer>(ent_ducky, *mesh_ducky, *mat_ducky);
	ent_ducky.Add<CPathAnimator>(ent_ducky);
	ent_ducky.transform.m_scale = glm::vec3(0.005f, 0.005f, 0.005f);
	ent_ducky.transform.m_pos = glm::vec3(0.0f, -1.0f, 0.0f);
	ent_ducky.transform.m_rotation = glm::angleAxis(glm::radians(-30.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	// Set up waypoint container
	std::vector<std::unique_ptr<Entity>> points;

	// Setting up path sampler
	PathSampler sampler = PathSampler();

	Entity pathDrawUtility = Entity::Create();
	pathDrawUtility.Add<CLineRenderer>(pathDrawUtility, sampler, *mat_line);

	App::Tick();

	float test1 = 0.f, test2 = 0.f, test3 = 0.f;

	// Main loop
	while (!App::IsClosing() && !Input::GetKeyDown(GLFW_KEY_ESCAPE))
	{
		App::FrameStart();

		// Update our LERP timers
		float deltaTime = App::GetDeltaTime();

		// Update camera
		ent_camera.Get<CCamera>().Update();

		// Update duck's path animator
		ent_ducky.Get<CPathAnimator>().Update(points, deltaTime);

		// Update transforms on all our points
		for (int i = 0; i < points.size(); i++)
		{
			points[i]->transform.RecomputeGlobal();
		}

		// Update duck transform
		ent_ducky.transform.RecomputeGlobal();

		// Draw our points
		for (int i = 0; i < points.size(); i++)
		{
			points[i]->Get<CMeshRenderer>().Draw();
		}

		// Draw duck
		ent_ducky.Get<CMeshRenderer>().Draw();

		// Draw line renderer
		pathDrawUtility.Get<CLineRenderer>().Draw(points);

		App::StartImgui();

		static bool listPanel = true;
		ImGui::Begin("Waypoint editor", &listPanel, ImVec2(300, 200));
		static size_t pointSelected = 0;
		static std::string pointLabel = "";

		// TODO: add an Imgui button to create new points
		if (ImGui::Button("Add"))
		{
			points.push_back(Entity::Allocate());
			points.back()->Add<CMeshRenderer>(*points.back(), *mesh_box, *mat_unselected);
		}
		// TODO: add an Imgui button to delete the selected point
		if (ImGui::Button("Delete Selected"))
		{
			std::cout << "Removed point 0" << std::endl;
			points.erase(points.begin() + pointSelected);

		}
		//Interface for selecting a waypoint.
		

		if (pointSelected >= points.size())
			pointSelected = points.size() - 1;

		for (size_t i = 0; i < points.size(); ++i)
		{
			pointLabel = "Point " + std::to_string(i);

			if (ImGui::Selectable(pointLabel.c_str(), i == pointSelected))
			{
				if (pointSelected < points.size())
					points[pointSelected]->Get<CMeshRenderer>().SetMaterial(*mat_unselected);

				pointSelected = i;
			}
		}

		ImGui::End();
		/*
		bool transformPanel = true;

		ImGui::Begin("Point Coordinates", &transformPanel, ImVec2(300, 100));

		// TODO: find a way to change the selected point's position via Imgui

		ImGui::InputFloat("x", &test1);
		ImGui::InputFloat("Y", &test2);
		ImGui::InputFloat("Z", &test3);

		std::cout << test1 << " " << test2 << " " << test3 << std::endl;

		ImGui::End();
		*/
		//Interface for moving a selected component.
		if (pointSelected < points.size())
		{
			auto& transform = points[pointSelected]->transform;

			points[pointSelected]->Get<CMeshRenderer>().SetMaterial(*mat_selected);
			static bool transformPanel = true;

			ImGui::Begin("Point Coordinates", &transformPanel, ImVec2(300, 100));

			// TODO: find a way to change the selected point's position via Imgui

			ImGui::DragFloat("X", &transform.m_pos.x, 0.1f);
			ImGui::DragFloat("Y", &transform.m_pos.y, 0.1f);
			ImGui::DragFloat("Z", &transform.m_pos.z, 0.1f);

			std::cout << test1 << " " << test2 << " " << test3 << std::endl;

			ImGui::End();

		}

		App::EndImgui();

		// Draw everything we queued up to the screen
		App::SwapBuffers();
	}

	// Destroy window
	App::Cleanup();

	return 0;
}

void LoadDefaultResources()
{
	// Load in the shaders we will need and activate them
	// Textured lit shader
	std::unique_ptr vs_texLitShader = std::make_unique<Shader>("shaders/texturedlit.vert", GL_VERTEX_SHADER);
	std::unique_ptr fs_texLitShader = std::make_unique<Shader>("shaders/texturedlit.frag", GL_FRAGMENT_SHADER);
	std::vector<Shader*> texLit = { vs_texLitShader.get(), fs_texLitShader.get() };
	prog_texLit = std::make_unique<ShaderProgram>(texLit);

	// Untextured lit shader
	std::unique_ptr vs_litShader = std::make_unique<Shader>("shaders/lit.vert", GL_VERTEX_SHADER);
	std::unique_ptr fs_litShader = std::make_unique<Shader>("shaders/lit.frag", GL_FRAGMENT_SHADER);
	std::vector<Shader*> lit = { vs_litShader.get(), fs_litShader.get() };
	prog_lit = std::make_unique<ShaderProgram>(lit);

	// Untextured unlit shader
	std::unique_ptr vs_unlitShader = std::make_unique<Shader>("shaders/unlit.vert", GL_VERTEX_SHADER);
	std::unique_ptr fs_unlitShader = std::make_unique<Shader>("shaders/unlit.frag", GL_FRAGMENT_SHADER);
	std::vector<Shader*> unlit = { vs_unlitShader.get(), vs_unlitShader.get() };
	prog_unlit = std::make_unique<ShaderProgram>(unlit);

	// Set up duck
	mesh_ducky = std::make_unique<Mesh>();
	GLTF::LoadMesh("duck/Duck.gltf", *mesh_ducky);
	tex2D_ducky = std::make_unique<Texture2D>("duck/DuckCM.png");

	// Set up box
	mesh_box = std::make_unique<Mesh>();
	GLTF::LoadMesh("box/Box.gltf", *mesh_box);

	// Set up duck material
	mat_ducky = std::make_unique<Material>(*prog_texLit);
	mat_ducky->AddTexture("albedo", *tex2D_ducky);

	// Set up point and line materials
	mat_unselected = std::make_unique<Material>(*prog_lit);
	mat_unselected->m_color = glm::vec3(0.5f, 0.5f, 0.5f);

	mat_selected = std::make_unique<Material>(*prog_lit);
	mat_selected->m_color = glm::vec3(1.0f, 0.0f, 0.0f);

	mat_line = std::make_unique<Material>(*prog_unlit);
	mat_line->m_color = glm::vec3(1.0f, 1.0f, 1.0f);
}
