#pragma once
// Include ImGui so it will be visible when we include this file
#include <imgui.h>
#include "ResourceManager/IResource.h"
#include "ResourceManager/ResourceManager.h"

#include <map>
#include <EnumToString.h>
#include "Graphics/ShaderProgram.h"
#include "Graphics/Textures/Texture2D.h"

// Will be included in the CPP to avoid header bloat
struct GLFWwindow;


// Allows for an ImGui command to have a left aligned label instead of right aligned
// EX: LABEL_LEFT(ImGui::DragFloat3, "Label", &value);
#define LABEL_LEFT(func, label, ...) (ImGui::TextUnformatted(label), ImGui::SameLine(), func("##" label, __VA_ARGS__))

#define GET_ENUM_MAP(E) (impl::E##MapName)

#define ENUM_COMBO(label, value, TypeName) \
	ImGuiHelper::DrawEnumCombo(label, value, impl::TypeName##MapName);

/// <summary>
/// Helper class for working with ImGui
/// </summary>
class ImGuiHelper {
public:
	/// <summary>
	/// Initializes the ImGui helper, should be called before doing any ImGui stuff
	/// </summary>
	static void Init(GLFWwindow* window);
	/// <summary>
	/// Cleans up the ImGui helper, should be called before closing the application
	/// </summary>
	static void Cleanup();

	/// <summary>
	/// Presents a red button with the given text, and with an optional size
	/// </summary>
	/// <param name="text">The text on the button</param>
	/// <param name="size">The optional size of the button</param>
	/// <returns>True if the button was pressed, false if otherwise</returns>
	static bool WarningButton(const char* text, const ImVec2& size = ImVec2(0,0));

	template <typename T, typename BT>
	static bool DrawEnumCombo(const char* label, T* value, const std::map<BT, std::string>& nameMap) {
		bool result = false; 
		if (ImGui::BeginCombo(label, (~*value).c_str())) {
			bool selected = false;
			for (const auto& [key, str] : nameMap) {
				selected = (*value == (T)key);
				if (ImGui::Selectable(str.c_str(), &selected)) {

					*value = (T)key;
					result = true;
				}
			}
			ImGui::EndCombo();
		}
		return result;
	}

	static void ResourceDragSource(const IResource* resource, const std::string& name);

	template <typename T>
	static bool ResourceDragTarget(std::shared_ptr<T>& resourceOut) {
		std::string typeName = StringTools::SanitizeClassName(typeid(T).name());
		bool result = false;

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(typeName.c_str())) {
				Guid guid = *(Guid*)payload->Data;
				resourceOut = ResourceManager::Get<T>(guid);
				result = true;
			}
			ImGui::EndDragDropTarget();
		}

		return result;
	}

	static bool DrawTextureDrop(Texture2D::Sptr& image, ImVec2 size);

	static void DrawLinearDepthTexture(const Texture2D::Sptr& image, const glm::ivec2& size, float zNear, float zFar);

	/// <summary>
	/// Notifies ImGui that a new frame has begun
	/// Call at start of render loop
	/// </summary>
	static void StartFrame();

	/// <summary>
	/// Notifies ImGui that a frame has ended and should be rendered
	/// Call at end of render loop before glfwSwapBuffers
	/// </summary>
	static void EndFrame();

	static void HeaderCheckbox(ImGuiID headerId, bool* value);

protected:
	ImGuiHelper() = default;

	static GLFWwindow* _window;

	static ShaderProgram::Sptr _linearDepthShader;
};