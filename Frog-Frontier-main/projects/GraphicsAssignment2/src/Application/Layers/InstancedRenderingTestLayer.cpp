#include "InstancedRenderingTestLayer.h"
#include "Utils/ObjLoader.h"
#include "Gameplay/Scene.h"
#include "Application/Application.h"
#include "Gameplay/Components/RotatingBehaviour.h"

InstancedRenderingTestLayer::InstancedRenderingTestLayer()
	: ApplicationLayer()
{
	Name = "Instanced Rendering";
	Overrides = AppLayerFunctions::OnSceneLoad | AppLayerFunctions::OnRender | AppLayerFunctions::OnUpdate;
}

InstancedRenderingTestLayer::~InstancedRenderingTestLayer()
{ }

void InstancedRenderingTestLayer::OnSceneLoad() {
	Gameplay::Scene::Sptr scene = Application::Get().CurrentScene();

	// The number of elements we're generating as a cube
	const glm::ivec3 size = { 10, 10, 10 };
	float distance = 2.0f;   

	// Create a buffer to store out data, and reserve enough space for all our instances
	_instanceBuffer = VertexBuffer::Create(BufferUsage::DynamicDraw);
	_instanceBuffer->LoadData<InstanceInfo>(nullptr, size.x * size.y * size.z);

	// Sending our 2 matrices as attributes
	std::vector<BufferAttribute> instancedParams = {
		BufferAttribute(8,  4, AttributeType::Float, sizeof(InstanceInfo), 0, AttribUsage::User0),
		BufferAttribute(9,  4, AttributeType::Float, sizeof(InstanceInfo), 4 * sizeof(float), AttribUsage::User0),
		BufferAttribute(10, 4, AttributeType::Float, sizeof(InstanceInfo), 8 * sizeof(float), AttribUsage::User0),
		BufferAttribute(11, 4, AttributeType::Float, sizeof(InstanceInfo), 12 * sizeof(float), AttribUsage::User0),

		BufferAttribute(12, 3, AttributeType::Float, sizeof(InstanceInfo), 16 * sizeof(float), AttribUsage::User0),
		BufferAttribute(13, 3, AttributeType::Float, sizeof(InstanceInfo), 20 * sizeof(float), AttribUsage::User0),
		BufferAttribute(14, 3, AttributeType::Float, sizeof(InstanceInfo), 24 * sizeof(float), AttribUsage::User0),
	};

	// Load a file to get the base VAO, then add the instanced buffers
	_vao = ObjLoader::LoadFromFile("monkey.obj");
	_vao->AddVertexBuffer(_instanceBuffer, instancedParams, true);

	// Load our instanced shader
	_shader = ShaderProgram::Create();
	_shader->LoadShaderPartFromFile("shaders/vertex_shaders/basic_instanced.glsl", ShaderPartType::Vertex); 
	_shader->LoadShaderPartFromFile("shaders/fragment_shaders/frag_environment_mirror.glsl", ShaderPartType::Fragment);
	_shader->Link();


	// Due to how scene stuff is handled in editor, we'll remove all existing instances and re-add them
	for (auto& instance : _instances) {
		scene->RemoveGameObject(scene->FindObjectByGUID(instance));
	}

	// Create a bunch of instances in a cube
	_instances.clear();
	_instances.reserve(size.x * size.y * size.z);
	for (int ix = 0; ix < size.x; ix++) {
		for (int iy = 0; iy < size.y; iy++) {
			for (int iz = 0; iz < size.z; iz++) {
				Gameplay::GameObject::Sptr instance = scene->CreateGameObject("Instanced");
				instance->SetPostion({ ix * distance, iy * distance, iz * distance });
				instance->HideInHierarchy = true;
				instance->Add<RotatingBehaviour>()->RotationSpeed = { 
					(rand() / (float)RAND_MAX) * 90.0f, 
					0,
					(rand() / (float)RAND_MAX) * 90.0f
				};
				_instances.push_back(instance);
			}
		}
	}

	// Upload instance data
	_UpdateInstances();
}

void InstancedRenderingTestLayer::OnUpdate() {
	//_UpdateInstances();
}

void InstancedRenderingTestLayer::OnRender(const Framebuffer::Sptr& prevLayer) {
	_shader->Bind();
	_vao->DrawInstanced(_instances.size());
}

void InstancedRenderingTestLayer::_UpdateInstances() {
	// We map our data into CPU-accessible memory, then cast it to our structure
	InstanceInfo* data = reinterpret_cast<InstanceInfo*>(_instanceBuffer->Map(BufferMapMode::Write));

	// Iterate over all instances
	for (int ix = 0; ix < _instances.size(); ix++) {
		// For now just update everything regardless of if it's changed or not
		// A smarter system would only update if the data is old
		data[ix].ModelMatrix  = _instances[ix]->GetTransform();
		data[ix].NormalMatrix = glm::mat3(glm::transpose(glm::inverse(_instances[ix]->GetTransform())));
	}

	// Unmap the buffer so that the GPU can see it again
	_instanceBuffer->Unmap();
}
