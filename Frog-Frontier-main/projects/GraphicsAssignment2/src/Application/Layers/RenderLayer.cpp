#include "RenderLayer.h"
#include "../Application.h"
#include "Graphics/GuiBatcher.h"
#include "Gameplay/Components/Camera.h"
#include "Graphics/DebugDraw.h"
#include "Graphics/Textures/TextureCube.h"
#include "../Timing.h"
#include "Gameplay/Components/ComponentManager.h"
#include "Gameplay/Components/RenderComponent.h"
#include "Gameplay/Components/Light.h"

// GLM math library
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <GLM/gtx/common.hpp> // for fmod (floating modulus)
#include "Gameplay/Components/ShadowCamera.h"


RenderLayer::RenderLayer() :
	ApplicationLayer(),
	_primaryFBO(nullptr),
	_blitFbo(true),
	_frameUniforms(nullptr),
	_instanceUniforms(nullptr),
	_renderFlags(RenderFlags::None),
	_clearColor({ 0.1f, 0.1f, 0.1f, 1.0f })
{
	Name = "Rendering";
	Overrides = 
		AppLayerFunctions::OnAppLoad | 
		AppLayerFunctions::OnPreRender | AppLayerFunctions::OnRender | AppLayerFunctions::OnPostRender | 
		AppLayerFunctions::OnWindowResize;
}

RenderLayer::~RenderLayer() = default;

void RenderLayer::OnPreRender()
{
	using namespace Gameplay;

	Application& app = Application::Get();

	// Clear the color and depth buffers
	const glm::vec4 colors[4] = {
		glm::vec4(0.0f),
		glm::vec4(0.5f, 0.5f, 0.5f, 0.0f),
		glm::vec4(0.0f),
		glm::vec4(0.0f)
	};

	_primaryFBO->Bind();
	// Clear the framebuffer. Note that this also binds and sets the viewport
	_ClearFramebuffer(_primaryFBO, colors, 4);

	
	// Grab shorthands to the camera and shader from the scene
	Camera::Sptr camera = app.CurrentScene()->MainCamera;

	// Cache the camera's viewprojection
	glm::mat4 viewProj = camera->GetViewProjection();
	DebugDrawer::Get().SetViewProjection(viewProj);

	// The current material that is bound for rendering
	Material::Sptr currentMat = nullptr;
	ShaderProgram::Sptr shader = nullptr;

	// Bind the skybox texture to a reserved texture slot
	// See Material.h and Material.cpp for how we're reserving texture slots
	TextureCube::Sptr environment = app.CurrentScene()->GetSkyboxTexture();
	if (environment) {
		environment->Bind(15);
	}

	// Binding the color correction LUT
	Texture3D::Sptr colorLUT = app.CurrentScene()->GetColorLUT();
	if (colorLUT) {
		colorLUT->Bind(14);
	}

	// Here we'll bind all the UBOs to their corresponding slots
	_frameUniforms->Bind(FRAME_UBO_BINDING);
	_instanceUniforms->Bind(INSTANCE_UBO_BINDING);
	_lightingUbo->Bind(LIGHTING_UBO_BINDING);

	// Draw physics debug
	app.CurrentScene()->DrawPhysicsDebug();

	_InitFrameUniforms();
}

void RenderLayer::OnRender(const Framebuffer::Sptr& prevLayer)
{
	using namespace Gameplay;

	Application& app = Application::Get();
	
	// Make sure depth testing and culling are re-enabled
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE); 
	glDepthMask(true); 

	// Disable blending, we want to override any existing colors
	glDisable(GL_BLEND);

	// Grab shorthands to the camera and shader from the scene
	Camera::Sptr camera = app.CurrentScene()->MainCamera;

	// We can now render all our scene elements via the helper function
	_RenderScene(camera->GetView(), camera->GetProjection());

	// Use our cubemap to draw our skybox
	app.CurrentScene()->DrawSkybox();

	VertexArrayObject::Unbind(); 
}

void RenderLayer::OnPostRender() {
	using namespace Gameplay;

	// Unbind our G-Buffer
	_primaryFBO->Unbind(); 

	// Composite our lighting 
	_Composite();

	Application& app = Application::Get();
	const glm::uvec4& viewport = app.GetPrimaryViewport();

	// Restore viewport to game viewport
	glViewport(viewport.x, viewport.y, viewport.z, viewport.w);

	// Blit our depth to the primary framebuffer so that other rendering can use it
	glBlitNamedFramebuffer(
		_primaryFBO->GetHandle(), 0,
		0, 0, _primaryFBO->GetWidth(), _primaryFBO->GetHeight(),
		viewport.x, viewport.y, viewport.x + viewport.z, viewport.y + viewport.w,
		GL_DEPTH_BUFFER_BIT,
		GL_NEAREST
	);

	// TODO: post processing effects

	_outputBuffer->Unbind();
	_outputBuffer->Bind(FramebufferBinding::Read);
	Framebuffer::Blit(
		{ 0, 0, _outputBuffer->GetWidth(), _outputBuffer->GetHeight() },
		{ viewport.x, viewport.y, viewport.x + viewport.z, viewport.y + viewport.w },
		BufferFlags::Color
	);

	_outputBuffer->Unbind();
}

void RenderLayer::_AccumulateLighting()
{
	using namespace Gameplay;

	Application& app = Application::Get();
	Scene::Sptr& scene = app.CurrentScene();

	Camera::Sptr camera = app.CurrentScene()->MainCamera;
	const glm::mat4& view = camera->GetView();

	// Update our lighting UBO for any shaders that need it
	LightingUboStruct& data = _lightingUbo->GetData();
	data.AmbientCol = scene->GetAmbientLight();
	data.EnvironmentRotation = scene->GetSkyboxRotation() * glm::inverse(glm::mat3(scene->MainCamera->GetView()));

	const glm::vec3& ambient = scene->GetAmbientLight();
	const glm::vec4 colors[2] = {
		{ ambient, 1.0f },         // diffuse (multiplicative)
		{ 0.0f, 0.0f, 0.0f, 1.0f } // specular (additive)
	};
	_lightingFBO->Bind();
	_ClearFramebuffer(_lightingFBO, colors, 2);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE); 

	// Bind our shader for processing lighting 
	_lightAccumulationShader->Bind(); 

	// Bind our G-Buffer textures so that they're readable
	_primaryFBO->GetTextureAttachment(RenderTargetAttachment::Depth)->Bind(0);  // depth
	_primaryFBO->GetTextureAttachment(RenderTargetAttachment::Color0)->Bind(1); // albedo + spec
	_primaryFBO->GetTextureAttachment(RenderTargetAttachment::Color1)->Bind(2); // normals + metallic
	_primaryFBO->GetTextureAttachment(RenderTargetAttachment::Color2)->Bind(3); // emissive
	_primaryFBO->GetTextureAttachment(RenderTargetAttachment::Color3)->Bind(4); // view pos


	// Send in how many active lights we have and the global lighting settings
	data.AmbientCol = glm::vec3(0.1f);
	int ix = 0;
	app.CurrentScene()->Components().Each<Light>([&](const Light::Sptr& light) {
		// Get the light's position in view space, since we're doing view space lighting
		glm::vec4 pos = glm::vec4(light->GetGameObject()->GetWorldPosition(), 1.0f);
		pos = view * pos;

		// Copy to the ubo data
		data.Lights[ix].Position = (glm::vec3)(pos) / pos.w;
		data.Lights[ix].Intensity = light->GetIntensity();
		data.Lights[ix].Color = light->GetColor();
		data.Lights[ix].Attenuation = 1.0f / (1.0f + light->GetRadius());  

		ix++;

		// If we've reached the max # of lights the shader supports, draw to the screen and start the next batch
		if (ix == MAX_LIGHTS) {
			data.NumLights = MAX_LIGHTS;

			// Send updated data to OpenGL
			_lightingUbo->Update();

			// Draw the fullscreen quad to accumulate the lights
			_fullscreenQuad->Draw();

			ix = 0;
		}
	});

	// If we have lights left over that haven't been drawn, draw them now
	if (ix > 0) {
		data.NumLights = ix;

		// Send updated data to OpenGL
		_lightingUbo->Update();

		// Draw the fullscreen quad to accumulate the lights
		_fullscreenQuad->Draw();
	}

	// Re-render the scene for shadows
	app.CurrentScene()->Components().Each<ShadowCamera>([&](const ShadowCamera::Sptr& shadowCam) {
		// Bind the shadow camera's depth buffer and clear it
		shadowCam->GetDepthBuffer()->Bind();
		glClear(GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, shadowCam->GetBufferResolution().x, shadowCam->GetBufferResolution().y);

		_RenderScene(shadowCam->GetGameObject()->GetInverseTransform(), shadowCam->GetProjection());

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	});

	// Restore frame level uniforms
	_InitFrameUniforms();

	_lightingFBO->Bind();
	glViewport(0, 0, _lightingFBO->GetWidth(), _lightingFBO->GetHeight());

	// Bind our G-Buffer textures so that they're readable
	_primaryFBO->GetTextureAttachment(RenderTargetAttachment::Depth)->Bind(0);  // depth
	_primaryFBO->GetTextureAttachment(RenderTargetAttachment::Color0)->Bind(1); // albedo + spec
	_primaryFBO->GetTextureAttachment(RenderTargetAttachment::Color1)->Bind(2); // normals + metallic
	_primaryFBO->GetTextureAttachment(RenderTargetAttachment::Color2)->Bind(3); // emissive
	_primaryFBO->GetTextureAttachment(RenderTargetAttachment::Color3)->Bind(4); // view pos

	// Bind shadow composite shader
	_shadowShader->Bind();

	// Add each shadow casting light to the lighting buffers
	app.CurrentScene()->Components().Each<ShadowCamera>([&](const ShadowCamera::Sptr& shadowCam) {
		// This gets us the light -> view space matrix, which we'll inverse to go from view space to light space
		glm::mat4 lightSpaceMatrix = camera->GetView() * shadowCam->GetGameObject()->GetTransform();

		// Or we have a matrix to go from view space to shadow space
		glm::mat4 viewToShadow = shadowCam->GetProjection() * glm::inverse(lightSpaceMatrix);

		// Calculate light's position and direction in view space
		glm::vec3 lightDirViewSpace = glm::mat3(lightSpaceMatrix) * glm::vec3(0, 0, -1.0f); 
		glm::vec3 lightPosViewSpace = lightSpaceMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

		// Bind depth and projection mask for reading, making sure not to stomp G-Buffer bindings
		shadowCam->GetDepthBuffer()->BindAttachment(RenderTargetAttachment::Depth, 5);
		if (shadowCam->GetProjectionMask() != nullptr) {
			shadowCam->GetProjectionMask()->Bind(6);
		}

		//_shadowShader->SetUniformMatrix("u_ClipToShadow", clipToShadow); 
		_shadowShader->SetUniformMatrix("u_ViewToShadow", viewToShadow); 

		// Get color and normalize it (strip the alpha)
		glm::vec4 color = shadowCam->GetColor();
		color *= color.w;

		_shadowShader->SetUniform("u_LightDirViewspace", lightDirViewSpace);
		_shadowShader->SetUniform("u_ShadowBias", shadowCam->Bias);
		_shadowShader->SetUniform("u_NormalBias", shadowCam->NormalBias);
		_shadowShader->SetUniform("u_Attenuation", 1/shadowCam->Range);
		_shadowShader->SetUniform("u_Intensity", shadowCam->Intensity);
		_shadowShader->SetUniform("u_LightColor", (glm::vec3)color);
		_shadowShader->SetUniform("u_LightPosViewspace", lightPosViewSpace);
		_shadowShader->SetUniform("u_ShadowFlags", *shadowCam->Flags);

		// Draw the fullscreen quad to accumulate the lights
		_fullscreenQuad->Draw();
	});

	// Unbind the lighting FBO so we can read its textures
	_lightingFBO->Unbind();
}

void RenderLayer::_Composite()
{
	using namespace Gameplay;
	Application& app = Application::Get();

	Scene::Sptr& scene = app.CurrentScene();

	_AccumulateLighting();

	// We want to switch to our compositing shader
	_compositingShader->Bind();

	// Switch rendering to output
	_outputBuffer->Bind();
	glViewport(0, 0, _outputBuffer->GetWidth(), _outputBuffer->GetHeight());

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Disable blending, we want to override any existing colors
	glDisable(GL_BLEND);

	// Bind our albedo and lighting buffers so we can composite a final scene
	_primaryFBO->GetTextureAttachment(RenderTargetAttachment::Color0)->Bind(0);
	_primaryFBO->GetTextureAttachment(RenderTargetAttachment::Color1)->Bind(1);
	_lightingFBO->GetTextureAttachment(RenderTargetAttachment::Color0)->Bind(2); 
	_lightingFBO->GetTextureAttachment(RenderTargetAttachment::Color1)->Bind(3);
	_primaryFBO->GetTextureAttachment(RenderTargetAttachment::Color2)->Bind(4);  
	_fullscreenQuad->Draw(); 

	// Re-enable depth testing
	glEnable(GL_DEPTH_TEST);

	// Blit our depth from primary FBO to our output depth buffer
	glBlitNamedFramebuffer(
		_primaryFBO->GetHandle(), _outputBuffer->GetHandle(),
		0, 0, _primaryFBO->GetWidth(), _primaryFBO->GetHeight(),
		0, 0, _outputBuffer->GetWidth(), _outputBuffer->GetHeight(),
		GL_DEPTH_BUFFER_BIT,
		GL_NEAREST
	);

	// Use our cubemap to draw our skybox
	scene->DrawSkybox();

	_outputBuffer->Unbind();
}

void RenderLayer::_ClearFramebuffer(Framebuffer::Sptr& buffer, const glm::vec4* colors, int layers) {
	// Make the entire buffer visible
	glViewport(0, 0, buffer->GetWidth(), buffer->GetHeight());
	// Disable depth testing
	glEnable(GL_DEPTH_TEST); 
	// Enable depth writing
	glDepthMask(true);
	// Disable blending, we want to override the colors
	glDisable(GL_BLEND);
	// Ignore existing depth
	glDepthFunc(GL_ALWAYS);

	// Bind the buffer so we're writing to it
	buffer->Bind();

	// Bind our clear shader, and draw a fullscreen quad with all the clear colors
	_clearShader->Bind();
	_clearShader->SetUniform<glm::vec4>("ClearColors", colors, layers);
	_fullscreenQuad->Draw();

	// Reset depth test function to default
	glDepthFunc(GL_LESS);
}

void RenderLayer::OnWindowResize(const glm::ivec2& oldSize, const glm::ivec2& newSize)
{
	if (newSize.x * newSize.y == 0) return;

	// Set viewport and resize our primary FBO and light accumulation FBO
	_primaryFBO->Resize(newSize);
	_lightingFBO->Resize(newSize);
	_outputBuffer->Resize(newSize);

	// Update the main camera's projection
	Application& app = Application::Get();
	app.CurrentScene()->MainCamera->ResizeWindow(newSize.x, newSize.y);
}

void RenderLayer::OnAppLoad(const nlohmann::json& config)
{
	Application& app = Application::Get();

	// GL states, we'll enable depth testing and backface fulling
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// Create a new descriptor for our FBO
	FramebufferDescriptor fboDescriptor;
	fboDescriptor.Width = app.GetWindowSize().x;
	fboDescriptor.Height = app.GetWindowSize().y;

	// We want to use a 32 bit depth buffer, we'll ignore the stencil buffer for now
	fboDescriptor.RenderTargets[RenderTargetAttachment::Depth] = RenderTargetDescriptor(RenderTargetType::Depth32);
	// Color layer 0 (albedo, specular)
	fboDescriptor.RenderTargets[RenderTargetAttachment::Color0] = RenderTargetDescriptor(RenderTargetType::ColorRgba8);
	// Color layer 1 (normals, metallic)
	fboDescriptor.RenderTargets[RenderTargetAttachment::Color1] = RenderTargetDescriptor(RenderTargetType::ColorRgba8);
	// Color layer 2 (emissive)  
	fboDescriptor.RenderTargets[RenderTargetAttachment::Color2] = RenderTargetDescriptor(RenderTargetType::ColorRgba8);
	// Color layer 3 (view space position)  
	fboDescriptor.RenderTargets[RenderTargetAttachment::Color3] = RenderTargetDescriptor(RenderTargetType::ColorRgba16F);
	 
	// Create the primary FBO
	_primaryFBO = std::make_shared<Framebuffer>(fboDescriptor);

	fboDescriptor.RenderTargets.clear();
	fboDescriptor.RenderTargets[RenderTargetAttachment::Color0] = RenderTargetDescriptor(RenderTargetType::ColorRgba8); // Diffuse
	fboDescriptor.RenderTargets[RenderTargetAttachment::Color1] = RenderTargetDescriptor(RenderTargetType::ColorRgba8); // Specular

	_lightingFBO = std::make_shared<Framebuffer>(fboDescriptor);

	// Create an FBO to store final output
	fboDescriptor.RenderTargets.clear();
	fboDescriptor.RenderTargets[RenderTargetAttachment::Depth] = RenderTargetDescriptor(RenderTargetType::Depth32);
	fboDescriptor.RenderTargets[RenderTargetAttachment::Color0] = RenderTargetDescriptor(RenderTargetType::ColorRgba8);

	_outputBuffer = std::make_shared<Framebuffer>(fboDescriptor);

	// We'll use one shader for light accumulation for now
	_lightAccumulationShader = ShaderProgram::Create();
	_lightAccumulationShader->LoadShaderPartFromFile("shaders/vertex_shaders/fullscreen_quad.glsl", ShaderPartType::Vertex);
	_lightAccumulationShader->LoadShaderPartFromFile("shaders/fragment_shaders/light_accumulation.glsl", ShaderPartType::Fragment);
	_lightAccumulationShader->Link();

	_compositingShader = ShaderProgram::Create();
	_compositingShader->LoadShaderPartFromFile("shaders/vertex_shaders/fullscreen_quad.glsl", ShaderPartType::Vertex);
	_compositingShader->LoadShaderPartFromFile("shaders/fragment_shaders/deferred_composite.glsl", ShaderPartType::Fragment);
	_compositingShader->Link();

	_clearShader = ShaderProgram::Create();
	_clearShader->LoadShaderPartFromFile("shaders/vertex_shaders/fullscreen_quad.glsl", ShaderPartType::Vertex);
	_clearShader->LoadShaderPartFromFile("shaders/fragment_shaders/clear.glsl", ShaderPartType::Fragment);
	_clearShader->Link();

	_shadowShader = ShaderProgram::Create();
	_shadowShader->LoadShaderPartFromFile("shaders/vertex_shaders/fullscreen_quad.glsl", ShaderPartType::Vertex);
	_shadowShader->LoadShaderPartFromFile("shaders/fragment_shaders/shadow_composite.glsl", ShaderPartType::Fragment);
	_shadowShader->Link();

	// We need a mesh for drawing fullscreen quads

	glm::vec2 positions[6] = {
		{ -1.0f,  1.0f }, { -1.0f, -1.0f }, { 1.0f, 1.0f },
		{ -1.0f, -1.0f }, {  1.0f, -1.0f }, { 1.0f, 1.0f }
	};

	VertexBuffer::Sptr vbo = std::make_shared<VertexBuffer>();
	vbo->LoadData(positions, 6);

	_fullscreenQuad = VertexArrayObject::Create();
	_fullscreenQuad->AddVertexBuffer(vbo, {
		BufferAttribute(0, 2, AttributeType::Float, sizeof(glm::vec2), 0, AttribUsage::Position)
	});

	// Create our common uniform buffers
	_frameUniforms = std::make_shared<UniformBuffer<FrameLevelUniforms>>(BufferUsage::DynamicDraw);
	_instanceUniforms = std::make_shared<UniformBuffer<InstanceLevelUniforms>>(BufferUsage::DynamicDraw);
	_lightingUbo = std::make_shared<UniformBuffer<LightingUboStruct>>(BufferUsage::DynamicDraw);
}

const Framebuffer::Sptr& RenderLayer::GetPrimaryFBO() const {
	return _primaryFBO;
}

bool RenderLayer::IsBlitEnabled() const {
	return false;
}

void RenderLayer::SetBlitEnabled(bool value) {
	_blitFbo = value;
}

const Framebuffer::Sptr& RenderLayer::GetRenderOutput() const {
	return _outputBuffer;
}

const glm::vec4& RenderLayer::GetClearColor() const {
	return _clearColor;
}

void RenderLayer::SetClearColor(const glm::vec4 & value) {
	_clearColor = value;
}

void RenderLayer::SetRenderFlags(RenderFlags value) {
	_renderFlags = value;
}

RenderFlags RenderLayer::GetRenderFlags() const {
	return _renderFlags;
}

const Framebuffer::Sptr& RenderLayer::GetLightingBuffer() const {
	return _lightingFBO;
}

const Framebuffer::Sptr& RenderLayer::GetGBuffer() const
{
	return _primaryFBO;
}

void RenderLayer::_InitFrameUniforms()
{
	using namespace Gameplay;

	Application& app = Application::Get();

	// Grab shorthands to the camera and shader from the scene
	Camera::Sptr camera = app.CurrentScene()->MainCamera;

	// Cache the camera's viewprojection
	glm::mat4 viewProj = camera->GetViewProjection();
	glm::mat4 view = camera->GetView();

	// Upload frame level uniforms
	auto& frameData = _frameUniforms->GetData();
	frameData.u_Projection = camera->GetProjection();
	frameData.u_View = camera->GetView();
	frameData.u_ViewProjection = camera->GetViewProjection();
	frameData.u_CameraPos = glm::vec4(camera->GetGameObject()->GetPosition(), 1.0f);
	frameData.u_Time = static_cast<float>(Timing::Current().TimeSinceSceneLoad());
	frameData.u_DeltaTime = Timing::Current().DeltaTime();
	frameData.u_RenderFlags = _renderFlags;
	frameData.u_ZNear = camera->GetNearPlane();
	frameData.u_ZFar = camera->GetFarPlane();
	_frameUniforms->Update();
}

void RenderLayer::_RenderScene(const glm::mat4& view, const glm::mat4& projection)
{
	using namespace Gameplay;

	Application& app = Application::Get();

	glm::mat4 viewProj = projection * view;

	// The current material that is bound for rendering
	Material::Sptr currentMat = nullptr;
	ShaderProgram::Sptr shader = nullptr;

	Material::Sptr defaultMat = app.CurrentScene()->DefaultMaterial;

	auto& frameData = _frameUniforms->GetData();
	frameData.u_Projection = projection;
	frameData.u_View = view;
	frameData.u_ViewProjection = viewProj;
	frameData.u_CameraPos = view * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	_frameUniforms->Update();

	// Render all our objects
	app.CurrentScene()->Components().Each<RenderComponent>([&](const RenderComponent::Sptr& renderable) {
		// Early bail if mesh not set
		if (renderable->GetMesh() == nullptr) {
			return;
		}

		// If we don't have a material, try getting the scene's fallback material
		// If none exists, do not draw anything
		if (renderable->GetMaterial() == nullptr) {
			if (defaultMat != nullptr) {
				renderable->SetMaterial(defaultMat);
			}
			else {
				return;
			}
		}

		// If the material has changed, we need to bind the new shader and set up our material and frame data
		// Note: This is a good reason why we should be sorting the render components in ComponentManager
		if (renderable->GetMaterial() != currentMat) {
			currentMat = renderable->GetMaterial();
			shader = currentMat->GetShader();

			shader->Bind();
			currentMat->Apply();
		}

		// Grab the game object so we can do some stuff with it
		GameObject* object = renderable->GetGameObject();

		// Use our uniform buffer for our instance level uniforms
		auto& instanceData = _instanceUniforms->GetData();
		instanceData.u_Model = object->GetTransform();
		instanceData.u_ModelViewProjection = viewProj * object->GetTransform();
		instanceData.u_ModelView = view * object->GetTransform();
		instanceData.u_NormalMatrix = glm::mat3(glm::transpose(glm::inverse(object->GetTransform())));
		_instanceUniforms->Update();

		// Draw the object
		renderable->GetMesh()->Draw();

	});

}

