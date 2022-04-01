#pragma once

#include <memory>
#include <GLM/glm.hpp>
#include "Gameplay/Components/IComponent.h"

namespace Gameplay {
	/// <summary>
	/// Represents a simple perspective camera for use by first person or third person games
	/// </summary>
	class Camera : public IComponent
	{
	public:
		typedef std::shared_ptr<Camera> Sptr;

		inline static Sptr Create() {
			return std::make_shared<Camera>();
		}

	// IComponent implementation
	public:
		virtual void RenderImGui() override;

		MAKE_TYPENAME(Camera);

		virtual nlohmann::json ToJson() const override;
		static Camera::Sptr FromJson(const nlohmann::json& data);


	public:
		Camera();
		virtual ~Camera() = default;

		/// <summary>
		/// Notifies this camera that the window has resized, and updates our projection matrix
		/// </summary>
		/// <param name="windowWidth">The new width of the window</param>
		/// <param name="windowHeight">The new height of the window</param>
		void ResizeWindow(int windowWidth, int windowHeight);

		/// <summary>
		/// Sets wheter the camera uses an orthographic or perspective projection
		/// </summary>
		/// <param name="value">True for orthographic, false for perspective</param>
		void SetOrthoEnabled(bool value);

		/// <summary>
		/// Sets the vertical field of view in radians for this camera
		/// </summary>
		void SetFovRadians(float value);
		/// <summary>
		/// Sets the vertical field of view in degrees for this camera
		/// </summary>
		void SetFovDegrees(float value);

		/// <summary>
		/// Sets the scale of the orthographic projection along the vertical axis. If you want 1 unit = 1 pixel, this would be the window height
		/// </summary>
		/// <param name="value">The new value for the ortho vertical scale</param>
		void SetOrthoVerticalScale(float value);

		/// <summary>
		/// Gets the vertical scale of this camera when in projection mode (ie. how many units appear vertically within the window)
		/// </summary>
		float GetOrthoVerticalScale() const { return _orthoVerticalScale; }
		/// <summary>
		/// Gets whether this camera is in orthographic mode
		/// </summary>
		bool GetOrthoEnabled() const { return _isOrtho; }

		/// <summary>
		/// Gets the view matrix for this camera
		/// </summary>
		const glm::mat4& GetView() const;
		/// <summary>
		/// Gets the projection matrix for this camera
		/// </summary>
		const glm::mat4& GetProjection() const;
		/// <summary>
		/// Gets the combined view-projection matrix for this camera, calculating if needed
		/// </summary>
		const glm::mat4& GetViewProjection() const;

		const glm::vec4& GetClearColor() const;
		void SetClearColor(const glm::vec4& color);

		float GetNearPlane() const;
		float GetFarPlane() const;

		void SetNearPlane(float value);
		void SetFarPlane(float value);

	protected:
		float _nearPlane;
		float _farPlane;
		float _fovRadians;
		float _aspectRatio;
		float _orthoVerticalScale;
		bool _isOrtho;
		mutable bool _isProjectionDirty;

		glm::mat4 _view;
		mutable glm::mat4 _projection;

		// The view projection, it is mutable so we can re-calculate it during const methods
		mutable glm::mat4 _viewProjection;
		// A dirty flag that indicates whether we need to re-calculate our view projection matrix
		mutable bool      _isDirty;

		glm::vec4         _clearColor;

		// Recalculates the projection matrix
		const glm::mat4& __CalculateProjection() const;
	};
}