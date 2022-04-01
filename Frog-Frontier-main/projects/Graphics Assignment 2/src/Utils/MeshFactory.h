#pragma once
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include "MeshBuilder.h"
#include "Graphics/VertexTypes.h"
#include <json.hpp>

#include <EnumToString.h>

/// <summary>
/// Represent the type of object added by a MeshBuilderParam
/// </summary>
ENUM(MeshBuilderType, int, 
	 Unknown    = 0,
	 Plane      = 1,
	 Cube       = 2,
	 IcoShere   = 3,
	 UvSphere   = 4,
	 FaceInvert = 5
);

/// <summary>
/// Allows storing configuration parameters for mesh factory calls
/// </summary>
struct MeshBuilderParam {
	MeshBuilderType Type;
	std::unordered_map<std::string, glm::vec3> Params;
	glm::vec4 Color = glm::vec4(1.0f);

	static MeshBuilderParam CreateCube(const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& eulerDeg = glm::vec3(0.0f), const glm::vec4& col = glm::vec4(1.0f));
	static MeshBuilderParam CreateIcoSphere(const glm::vec3& center, float radius, int tessellation = 0, const glm::vec4& col = glm::vec4(1.0f));
	static MeshBuilderParam CreateIcoSphere(const glm::vec3& center, const glm::vec3& radii, int tessellation = 0, const glm::vec4& col = glm::vec4(1.0f));
	static MeshBuilderParam CreateUVSphere(const glm::vec3& center, float radius, int tessellation = 0, const glm::vec4& col = glm::vec4(1.0f));
	static MeshBuilderParam CreateUVSphere(const glm::vec3& center, const glm::vec3& radii, int tessellation = 0, const glm::vec4& col = glm::vec4(1.0f));
	static MeshBuilderParam CreatePlane(const glm::vec3& pos, const glm::vec3& normal, const glm::vec3& tangent, const glm::vec2& scale, const glm::vec2& uvScale = glm::vec2(1.0f), const glm::vec4& col = glm::vec4(1.0f));
	static MeshBuilderParam CreateInvert();

	static MeshBuilderParam FromJson(const nlohmann::json& blob);
	nlohmann::json   ToJson() const;
};


/// <summary>
/// Provides tools for generating meshes at runtime
/// </summary>
class MeshFactory
{
public:
	/// <summary>
	/// Adds a cube to the given mesh
	/// </summary>
	/// <typeparam name="Vertex">The type of vertex the mesh consists of</typeparam>
	/// <param name="mesh">The mesh to add the cube to</param>
	/// <param name="pos">The position of the center of the cube in object space</param>
	/// <param name="scale">The scaling to apply to the cube (by default, the cube is 1 unit cubed)</param>
	/// <param name="eulerDeg">The euler angles to rotate the cube by (in degrees)</param>
	/// <param name="col">The color of the cube to add</param>
	template <typename Vertex>
	static void AddCube(MeshBuilder<Vertex>& mesh, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& eulerDeg = glm::vec3(0.0f), const glm::vec4& col = glm::vec4(1.0f));

	/// <summary>
	/// Adds a cube to the given mesh using a transformation matrix to orient it in world space
	/// </summary>
	/// <typeparam name="Vertex">The type of vertex the mesh consists of</typeparam>
	/// <param name="mesh">The mesh to add the cube to</param>
	/// <param name="transform">The matrix to apply to a 1x1x1 cube centered around the origin</param>
	/// <param name="col">The color of the cube to add</param>
	template <typename Vertex>
	static void AddCube(MeshBuilder<Vertex>& mesh, const glm::mat4& transform, const glm::vec4& col = glm::vec4(1.0f));

	/// <summary>
	/// Adds an ico-sphere to this mesh, which is tessellated from a set of triangles
	/// </summary>
	/// <typeparam name="Vertex">The type of vertex the mesh consists of</typeparam>
	/// <param name="data">The mesh to add the cube to</param>
	/// <param name="center">The center of the sphere, in object space</param>
	/// <param name="radius">The radius of the sphere</param>
	/// <param name="tessellation">The number of tessellation steps to apply</param>
	/// <param name="col">The color of the sphere</param>
	template <typename Vertex>
	static void AddIcoSphere(MeshBuilder<Vertex>& data, const glm::vec3& center, float radius, int tessellation = 0, const glm::vec4& col = glm::vec4(1.0f));

	/// <summary>
	/// Adds an ico-sphere to this mesh, which is tessellated from a set of triangles, scaling it by variable radii
	/// </summary>
	/// <typeparam name="Vertex">The type of vertex the mesh consists of</typeparam>
	/// <param name="data">The mesh to add the cube to</param>
	/// <param name="center">The center of the sphere, in object space</param>
	/// <param name="radii">The radii of the sphere along each axis</param>
	/// <param name="tessellation">The number of tessellation steps to apply</param>
	/// <param name="col">The color of the sphere</param>
	template <typename Vertex>
	static void AddIcoSphere(MeshBuilder<Vertex>& data, const glm::vec3& center, const glm::vec3& radii, int tessellation = 0, const glm::vec4& col = glm::vec4(1.0f));

	/// <summary>
	/// Adds a UV sphere to the mesh, where tessellation indicates the number of subdivisions
	/// along the latitude/longitude (starting from 3 subdivisions)
	/// </summary>
	/// <typeparam name="Vertex">The type of vertex the mesh consists of</typeparam>
	/// <param name="data">The mesh to add the cube to</param>
	/// <param name="center">The center of the sphere, in object space</param>
	/// <param name="radius">The radius of the sphere</param>
	/// <param name="tessellation">The number of tessellation steps to apply</param>
	/// <param name="col">The color of the sphere</param>
	template <typename Vertex>
	static void AddUvSphere(MeshBuilder<Vertex>& data, const glm::vec3& center, float radius, int tessellation = 0, const glm::vec4& col = glm::vec4(1.0f));

	/// <summary>
	/// Adds a UV sphere to the mesh, where tessellation indicates the number of subdivisions
	/// along the latitude/longitude (starting from 3 subdivisions), scaling it by variable radii
	/// </summary>
	/// <typeparam name="Vertex">The type of vertex the mesh consists of</typeparam>
	/// <param name="data">The mesh to add the cube to</param>
	/// <param name="center">The center of the sphere, in object space</param>
	/// <param name="radii">The radii of the sphere along each axis</param>
	/// <param name="tessellation">The number of tessellation steps to apply</param>
	/// <param name="col">The color of the sphere</param>
	template <typename Vertex>
	static void AddUvSphere(MeshBuilder<Vertex>& data, const glm::vec3& center, const glm::vec3& radii, int tessellation = 0, const glm::vec4& col = glm::vec4(1.0f));

	/// <summary>
	/// Adds a plane to the mesh, centered at a position and facing towards a normal and along a tangent
	/// </summary>
	/// <typeparam name="Vertex">The type of vertex the mesh consists of</typeparam>
	/// <param name="mesh">The mesh to add the cube to</param>
	/// <param name="pos">The center of the mesh in object space</param>
	/// <param name="normal">The direction that the plane is facing</param>
	/// <param name="tangent">The vector along which the local x-axis of the plane is aligned</param>
	/// <param name="scale">The size of the plane, with x being along tangent, and y being along normal X tangent</param>
	/// <param name="col">The color of the plane</param>
	template <typename Vertex>
	static void AddPlane(MeshBuilder<Vertex>& mesh, const glm::vec3& pos, const glm::vec3& normal, const glm::vec3& tangent, const glm::vec2& scale, const glm::vec2& uvScale = glm::vec2(1.0f), const glm::vec4& col = glm::vec4(1.0f));
	
	/// <summary>
	/// Adds a MeshBuilderParam instance to the given mesh
	/// </summary>
	/// <typeparam name="Vertex">The type of vertex the mesh consists of</typeparam>
	/// <param name="mesh">The mesh to add the object to</param>
	/// <param name="param">The mesh object parameters</param>
	template <typename Vertex>
	static void AddParameterized(MeshBuilder<Vertex>& mesh, const MeshBuilderParam& param);

	/// <summary>
	/// Manipulates a MeshBuilder by inverting all faces in the mesh
	/// </summary>
	/// <typeparam name="Vertex">The type of vertex the mesh consists of</typeparam>
	/// <param name="mesh">The mesh to manipulate</param>
	template <typename Vertex>
	static void InvertFaces(MeshBuilder<Vertex>& mesh);

	/// <summary>
	/// Calculates the tangents and bitangents from the normal and UV coords
	/// </summary>
	/// <typeparam name="Vertex">The type of vertex the mesh consists of</typeparam>
	/// <param name="mesh">The mesh to manipulate</param>
	template <typename Vertex>
	static void CalculateTBN(MeshBuilder<Vertex>& mesh);

protected:	
	MeshFactory() = default;
	~MeshFactory() = default;

	inline static const glm::mat4 MAT4_IDENTITY = glm::mat4(1.0f);
};

#include "MeshFactory.inl"