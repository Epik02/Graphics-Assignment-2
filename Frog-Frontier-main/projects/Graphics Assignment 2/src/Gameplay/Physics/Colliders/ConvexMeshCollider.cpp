#include "ConvexMeshCollider.h"
#include <BulletCollision/CollisionShapes/btShapeHull.h>

#include "Gameplay/GameObject.h"
#include "Gameplay/MeshResource.h"
#include "Gameplay/Components/RenderComponent.h"

#include "Utils/GlmBulletConversions.h"

namespace Gameplay::Physics {
	ConvexMeshCollider::Sptr ConvexMeshCollider::Create() {
		return std::shared_ptr<ConvexMeshCollider>(new ConvexMeshCollider());
	}

	ConvexMeshCollider::~ConvexMeshCollider() = default;

	ConvexMeshCollider::ConvexMeshCollider() :
		ICollider(ColliderType::ConvexMesh),
		_triMesh(nullptr)
	{ }

	btCollisionShape* ConvexMeshCollider::CreateShape() const {
		// https://pybullet.org/Bullet/phpBB3/viewtopic.php?t=4513
		if (_triMesh == nullptr) {
			return nullptr;
		}
		btConvexShape* result = new btConvexTriangleMeshShape(_triMesh);

		// The hull shape will calculate the convex hull that contains our shape
		//btShapeHull* hull = new btShapeHull(result);
		//float margin = result->getMargin();
		//bool build = hull->buildHull(margin);
		//if (!build) {
		//	LOG_WARN("Failed to build hull for convex mesh");
		//}
		//delete hull;

		return result;
	}

	void ConvexMeshCollider::Awake(GameObject* context)
	{
		// Get the components from the gameobject that we'll need to generate the mesh
		RenderComponent::Sptr renderer = context->Get<RenderComponent>();
		MeshResource::Sptr mesh = (renderer != nullptr ? renderer->GetMeshResource() : nullptr);

		// If we have no mesh, we can't create a collider for it!
		if (mesh == nullptr) {
			LOG_WARN("Mesh collider attached to gameobject without a mesh!");
			return;
		}

		// If we have an explicit collider, grab that instead
		if (mesh->ColliderMeshData != nullptr) {
			mesh = mesh->ColliderMeshData;
		}

		// We've already calculated the mesh, use existing
		if (mesh->BulletTriMesh != nullptr) {
			_triMesh = mesh->BulletTriMesh.get();
		}
		// We need to calculate the triangle mesh from the mesh data
		else {
			// Get the VAO from the mesh and make sure it exists
			VertexArrayObject::Sptr vao = mesh->Mesh;
			if (vao == nullptr) {
				LOG_WARN("Mesh resource not fully configured!");
				return;
			}

			// Get the vertex declaration from the VAO so we can pull out positions
			const VertexArrayObject::VertexDeclaration& VDecl = vao->GetVDecl();
			if (VDecl.size() == 0) {
				LOG_WARN("Mesh does not have a vertex declaration, unable to determine position elements");
				return;
			}

			// Get the attribute for positions from the vertex declaration
			auto& it = std::find_if(VDecl.begin(), VDecl.end(), [](const BufferAttribute& attrib) {
				return attrib.Usage == AttribUsage::Position;
			});
			if (it == VDecl.end()) {
				LOG_WARN("Mesh vertex declaration does not have a position element");
				return;
			}
			BufferAttribute posAttrib = *it;

			// Get the VBO that contains our data about the position elements
			const auto* vertBuff = vao->GetBufferBinding(AttribUsage::Position);
			if (vertBuff != nullptr) {
				// Shorthand our buffers
				IndexBuffer::Sptr indexBuff = vao->GetIndexBuffer();
				VertexBuffer::Sptr vertexBuff = vertBuff->GetBuffer();

				// Create the bullet physics triangle mesh
				_triMesh = new btTriangleMesh();

				// Helper for extracting an int from a raw index buffer datastore
				auto getBufferIndex = [](IndexBuffer::Sptr buff, uint8_t* dataStore, int offset) {
					switch (buff->GetElementType())
					{
						case IndexType::UByte:
							return (int)*(dataStore + offset);
						case IndexType::UShort:
							return (int)*(reinterpret_cast<uint16_t*>(dataStore) + offset);
						case IndexType::UInt:
							return (int)*(reinterpret_cast<uint32_t*>(dataStore) + offset);
						case IndexType::Unknown:
						default:
							return 0;
					}
				};

				// Allocate some space to read data from OpenGL and read our buffer data back into CPU memory
				uint8_t* vertexStore = reinterpret_cast<uint8_t*>(malloc(vertexBuff->GetTotalSize()));
				glGetNamedBufferSubData(vertexBuff->GetHandle(), 0, vertexBuff->GetTotalSize(), vertexStore);
				_triMesh->preallocateVertices(vao->GetVertexCount());

				// If our data is indexed, we use the index buffer to add our triangles
				if (indexBuff != nullptr) {
					// Allocate and read space for the indices
					uint8_t* indexStore = reinterpret_cast<uint8_t*>(malloc(indexBuff->GetTotalSize()));
					glGetNamedBufferSubData(indexBuff->GetHandle(), 0, indexBuff->GetTotalSize(), indexStore);

					// Iterate over index triangles
					for (size_t ix = 0; ix < indexBuff->GetElementCount(); ix+=3) {
						// Extract index from the raw data
						int i1 = getBufferIndex(indexBuff, indexStore, static_cast<int>(ix));
						int i2 = getBufferIndex(indexBuff, indexStore, static_cast<int>(ix + 1));
						int i3 = getBufferIndex(indexBuff, indexStore, static_cast<int>(ix + 2));

						// Find the positions for the indices
						glm::vec3 p1 = *reinterpret_cast<glm::vec3*>(vertexStore + (posAttrib.Stride * i1) + posAttrib.Offset);
						glm::vec3 p2 = *reinterpret_cast<glm::vec3*>(vertexStore + (posAttrib.Stride * i2) + posAttrib.Offset);
						glm::vec3 p3 = *reinterpret_cast<glm::vec3*>(vertexStore + (posAttrib.Stride * i3) + posAttrib.Offset);

						// Add the triangle
						_triMesh->addTriangle(ToBt(p1), ToBt(p2), ToBt(p3));
					}
			
					// Free the data we copied the indices into
					free(indexStore);

				}
				// We only have vertex data, create triangles sequentially
				else {
					// Iterate over triangles, and add each to the mesh
					for (size_t ix = 0; ix < vertexBuff->GetElementCount(); ix+=3) {
						glm::vec3 p1 = *reinterpret_cast<glm::vec3*>(vertexStore + ((ix + 0) * posAttrib.Stride) + posAttrib.Offset);
						glm::vec3 p2 = *reinterpret_cast<glm::vec3*>(vertexStore + ((ix + 1) * posAttrib.Stride) + posAttrib.Offset);
						glm::vec3 p3 = *reinterpret_cast<glm::vec3*>(vertexStore + ((ix + 2) * posAttrib.Stride) + posAttrib.Offset);
						_triMesh->addTriangle(ToBt(p1), ToBt(p2), ToBt(p3));
					}
				}

				// free our vertex store data
				free(vertexStore);

				// Store the bullet tri mesh in the MeshResource in case we want it later
				mesh->BulletTriMesh = std::shared_ptr<btTriangleMesh>(_triMesh);
			}
		}
	}

	void ConvexMeshCollider::FromJson(const nlohmann::json& data) {
	}

	void ConvexMeshCollider::ToJson(nlohmann::json& blob) const {
	}

	void ConvexMeshCollider::DrawImGui() {
	}
}
