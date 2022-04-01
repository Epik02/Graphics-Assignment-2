#pragma once
#include <EnumToString.h>
#include "glad/glad.h"
#include "Graphics/GlEnums.h"

/**
 * Represents the state of the OpenGL blend function 
 */
struct BlendState {
	/**
	 * True if blending is enabled, false if otherwise
	 */
	bool          BlendEnabled   = false;
	/**
	 * The blend function to use for the RGB channels
	 */
	BlendEquation RgbBlendFunc   = BlendEquation::Add;
	/**
	 * The blend function to use for the alpha channels
	 */
	BlendEquation AlphaBlendFunc = BlendEquation::Add;
	/**
	 * Blend function for source RGB
	 */
	BlendFunc     SrcRgb         = BlendFunc::One;
	/**
	 * Blend function for destination RGB
	 */
	BlendFunc     DstRgb         = BlendFunc::One;
	/**
	 * Blend function for source alpha
	 */
	BlendFunc     SrcAlpha       = BlendFunc::Zero;
	/**
	 * Blend function for destination alpha
	 */
	BlendFunc     DstAlpha       = BlendFunc::Zero;

	/**
	 * Applies this blending state to the OpenGL pipeline
	 */
	inline void Apply() {
		if (BlendEnabled) {
			glEnable(GL_BLEND);
			glBlendFuncSeparate(*SrcRgb, *DstRgb, *SrcAlpha, *DstAlpha);
			glBlendEquationSeparate(*RgbBlendFunc, *AlphaBlendFunc);
		}
		else  {
			glDisable(GL_BLEND);
		}
	}
};
	
/**
 * Defines a blend state that implements basic alpha blending
 */
const BlendState AlphaBlendState = {
	true,
	BlendEquation::Add,
	BlendEquation::Add,
	BlendFunc::One,
	BlendFunc::OneMinusSrcAlpha,
	BlendFunc::One,
	BlendFunc::One
};

/*
* Represents the core state of the graphics rasterizer, such as the culling, fill modes, blending, etc...
*/
struct RasterizerState {
	/**
	 * Polygon fill mode for front facing triangles
	 */
	FillMode FrontFaceFill = FillMode::Fill;
	/**
	 * Polygon fill mode for back facing triangles
	 */
	FillMode BackFaceFill  = FillMode::Fill;
	/**
	 * Culling mode (front face, back face, both, none)
	 */
	CullMode CullMode      = CullMode::Back;
	/**
	 * The blend state for this rasterizer state
	 */
	BlendState Blending    = BlendState();

	/**
	 * Applies the entire rasterizer state to the OpenGL render pipeline
	 */
	inline void Apply() {
		glPolygonMode(GL_FRONT, *FrontFaceFill);
		glPolygonMode(GL_BACK, *BackFaceFill);
		if (CullMode != CullMode::None) {
			glEnable(GL_CULL_FACE);
			glCullFace(*CullMode);
		} else {
			glDisable(GL_CULL_FACE);
		}
	}
};