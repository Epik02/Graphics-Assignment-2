#pragma once

#include "Application/ApplicationLayer.h"
#include "Utils/Macros.h"
#include "Graphics/VertexArrayObject.h"

/**
 * The post processing layer will handle rendering effects after the primary
 * deffered pipeline has composited an output image
 */
class PostProcessingLayer final : public ApplicationLayer {
public:
	MAKE_PTRS(PostProcessingLayer);

	/**
	 * Base class for post processing effects, we extend this to create new effects
	 */
	class Effect : public IResource {
	public:
		MAKE_PTRS(Effect);

		// True if this effect is enabled, false if otherwise
		bool Enabled = true;

		// The name of the effect as seen in the effects window
		std::string Name;

		virtual ~Effect() = default;

		/**
		 * Overload this in derived classes to apply the effect. Texture slot 0
		 * will contain the image from the previous pass
		 * @param gBuffer The G-Buffer from the deferred rendering pipeline
		 */
		virtual void Apply(const Framebuffer::Sptr& gBuffer) = 0;
		/**
		 * Allows this effect to perform logic when a new scene is loaded
		 */
		virtual void OnSceneLoad() {}
		/**
		 * Allows this effect to perform logic when a scene is unloaded
		 */
		virtual void OnSceneUnload() {}
		/**
		 * Allows this effect to perform additional logic when the window is resized
		 * Note that the output framebuffer will already be resized by the post processing layer
		 */
		virtual void OnWindowResize(const glm::ivec2& oldSize, const glm::ivec2& newSize) {}
		/**
		 * Allows this effect to render a custom set of ImGui controls in the effects window
		 */
		virtual void RenderImGui() {}

		/**
		 * Helper for drawing a fullscreen quad from within this effect. Uses
		 * state from the post processing layer, and cannot be used outside of
		 * that context
		 */
		void DrawFullscreen();

	protected:
		friend class PostProcessingLayer;

		// The output that this effect will render into
		Framebuffer::Sptr _output = nullptr;
		// The scaling between this effect's output and the screen size, default 1
		glm::vec2 _outputScale = glm::vec2(1);
		// The render target format for the effect's buffer
		RenderTargetType _format = RenderTargetType::ColorRgba8;
		
		Effect() = default;
	};

	PostProcessingLayer();
	virtual ~PostProcessingLayer();

	/**
	 * Gets the first effect of the given type from this post processing layer
	 */
	template <typename T, typename = typename std::enable_if<std::is_base_of<Effect, T>::value>::type>
	std::shared_ptr<T> GetEffect() {
		// Iterate over all the pointers in the binding list
		for (const auto& ptr : _effects) {
			// If the pointer type matches T, we return that behaviour, making sure to cast it back to the requested type
			if (std::type_index(typeid(*ptr.get())) == std::type_index(typeid(T))) {
				return std::dynamic_pointer_cast<T>(ptr);
			}
		}
		return nullptr;
	}

	/**
	 * Gets an immutable list of effects from this layer
	 */
	const std::vector<Effect::Sptr>& GetEffects() const;

	/**
	 * Adds a new effect to the end of the processing stack
	 */
	void AddEffect(const Effect::Sptr& effect);

	// Inherited from ApplicationLayer

	virtual void OnAppLoad(const nlohmann::json& config) override;
	virtual void OnPostRender() override;
	virtual void OnSceneLoad() override;
	virtual void OnSceneUnload() override;
	virtual void OnWindowResize(const glm::ivec2& oldSize, const glm::ivec2& newSize) override;

protected:
	friend class Effect;

	std::vector<Effect::Sptr> _effects;
	VertexArrayObject::Sptr _quadVAO;
};