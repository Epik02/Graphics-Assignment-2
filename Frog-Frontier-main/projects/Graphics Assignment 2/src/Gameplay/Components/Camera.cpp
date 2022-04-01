#include "Camera.h"

#include <GLM/gtc/matrix_transform.hpp>
#include "Utils/JsonGlmHelpers.h"
#include "Utils/GlmDefines.h"
#include "Gameplay/GameObject.h"
#include "Utils/ImGuiHelper.h"

namespace Gameplay {
	void Camera::RenderImGui()
	{
		_isProjectionDirty |= LABEL_LEFT(ImGui::DragFloat, "Near Plane", &_nearPlane, 0.001f, 0.001f, _farPlane);
		_isProjectionDirty |= LABEL_LEFT(ImGui::DragFloat, "Far Plane ", &_farPlane, 0.1f, _nearPlane);

		_isProjectionDirty |= ImGui::Checkbox("Is Ortho", &_isOrtho);

		if (_isOrtho) {
			_isProjectionDirty |= LABEL_LEFT(ImGui::DragFloat, "Ortho Scale", &_orthoVerticalScale, 0.01f, 0.01f);
		} else {
			float fov_deg = glm::degrees(_fovRadians);
			if (LABEL_LEFT(ImGui::SliderFloat, "FOV (deg) ", &fov_deg, 0.1f, 180.0f)) {
				_fovRadians = glm::radians(fov_deg);
				_isProjectionDirty |= true;
			}
		}
	}

	nlohmann::json Camera::ToJson() const
	{
		return {
			{ "near_plane", _nearPlane },
			{ "far_plane", _farPlane },
			{ "fov_radians", _fovRadians },
			{ "ortho_size", _orthoVerticalScale },
			{ "ortho_enabled", _isOrtho },
			{ "clear_color", _clearColor }
		};
	}

	Camera::Sptr Camera::FromJson(const nlohmann::json& data)
	{
		Camera::Sptr result = Camera::Create();
		result->_nearPlane          = JsonGet(data, "near_plane", result->_nearPlane);
		result->_farPlane           = JsonGet(data, "far_plane", result->_farPlane);
		result->_fovRadians         = JsonGet(data, "fov_radians", result->_fovRadians);
		result->_orthoVerticalScale = JsonGet(data, "ortho_size", result->_orthoVerticalScale);
		result->_isOrtho            = JsonGet(data, "ortho_enabled", result->_isOrtho);
		result->_clearColor         = JsonGet(data, "clear_color", result->_clearColor);
		result->_isProjectionDirty  = true;
		return result;
	}

	Camera::Camera() :
		_nearPlane(0.1f),
		_farPlane(1000.0f),
		_fovRadians(glm::radians(90.0f)),
		_aspectRatio(1.0f),
		_orthoVerticalScale(10.0f),
		_isOrtho(false),
		_view(glm::mat4(1.0f)),
		_projection(glm::mat4(1.0f)),
		_viewProjection(glm::mat4(1.0f)),
		_isDirty(true)
	{
		__CalculateProjection();
	}

	void Camera::ResizeWindow(int windowWidth, int windowHeight) {
		_aspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
		_isProjectionDirty = true;
	}

	void Camera::SetOrthoEnabled(bool value)
	{
		_isOrtho = true;
		_isProjectionDirty = true;
	}

	void Camera::SetFovRadians(float value) {
		_fovRadians = value;
		_isProjectionDirty = true;
	}

	void Camera::SetFovDegrees(float value) {
		SetFovRadians(glm::radians(value));
		_isProjectionDirty = true;
	}

	void Camera::SetOrthoVerticalScale(float value) {
		_orthoVerticalScale = value;
		_isProjectionDirty = true;
	}

	const glm::mat4& Camera::GetView() const {
		return GetGameObject()->GetInverseTransform();
	}

	const glm::mat4& Camera::GetProjection() const {
		__CalculateProjection();
		return _projection;
	}

	const glm::mat4& Camera::GetViewProjection() const {
		_viewProjection = __CalculateProjection() * GetGameObject()->GetInverseTransform();
		return _viewProjection;
	}

	const glm::vec4& Camera::GetClearColor() const
	{
		return _clearColor;
	}

	void Camera::SetClearColor(const glm::vec4& color) {
		_clearColor = color;
	}

	float Camera::GetNearPlane() const
	{
		return _nearPlane;
	}

	float Camera::GetFarPlane() const
	{
		return _farPlane;
	}

	void Camera::SetNearPlane(float value)
	{
		_nearPlane = value;
		__CalculateProjection();
	}

	void Camera::SetFarPlane(float value)
	{
		_farPlane = value;
		__CalculateProjection();
	}

	const glm::mat4& Camera::__CalculateProjection() const
	{
		if (_isProjectionDirty) {
			if (_isOrtho) {
				float w = (_orthoVerticalScale * _aspectRatio) / 2.0f;
				float h = (_orthoVerticalScale / 2.0f);
				_projection = glm::ortho(-w, w, -h, h, _nearPlane, _farPlane);
			} else {
				_projection = glm::perspective(_fovRadians, _aspectRatio, _nearPlane, _farPlane);
			}
			_isProjectionDirty = false;
		}
		return _projection;
	}
}