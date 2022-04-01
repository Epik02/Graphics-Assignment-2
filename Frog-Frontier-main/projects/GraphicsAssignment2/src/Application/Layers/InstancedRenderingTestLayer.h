#pragma once
#include "Application/ApplicationLayer.h"
#include <glad/glad.h>
#include <json.hpp>
#include "Graphics/VertexArrayObject.h"
#include "Graphics/ShaderProgram.h"
#include "Gameplay/GameObject.h"


/**
 * Handles demonstrating our instanced rendering
 */
class InstancedRenderingTestLayer final : public ApplicationLayer {
public:
	MAKE_PTRS(InstancedRenderingTestLayer)

	InstancedRenderingTestLayer();
	virtual ~InstancedRenderingTestLayer();

	// Inherited from ApplicationLayer

	virtual void OnSceneLoad() override;
	virtual void OnUpdate() override;
	virtual void OnRender(const Framebuffer::Sptr& prevLayer) override;

protected:
	VertexArrayObject::Sptr _vao;
	VertexBuffer::Sptr _instanceBuffer;
	ShaderProgram::Sptr _shader;
	Texture2D::Sptr     _texture;
		
	std::vector<Gameplay::GameObject::WeakRef> _instances;

	struct InstanceInfo {
		glm::mat4 ModelMatrix;
		glm::mat4 NormalMatrix;
	};

	void _UpdateInstances();
};