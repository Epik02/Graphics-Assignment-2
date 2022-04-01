#pragma once
#include <GLM/glm.hpp>
#include <stack>
#include "Graphics/VertexTypes.h"
#include "Graphics/ShaderProgram.h"

/// <summary>
/// Utility class for drawing lines and triangles in an immediate mode style
/// 
/// Includes a stack for transformations and color, to ease implementation of complex
/// debuggers
/// </summary>
class DebugDrawer
{
public:
	inline static const size_t LINE_BATCH_SIZE = 8192;
	inline static const size_t TRI_BATCH_SIZE = 4096;

	// Delete copy and mode

	DebugDrawer(const DebugDrawer& other) = delete;
	DebugDrawer(DebugDrawer&& other) = delete;
	DebugDrawer& operator =(const DebugDrawer& other) = delete;
	DebugDrawer& operator =(DebugDrawer&& other) = delete;

	virtual ~DebugDrawer() = default;

	/// <summary>
	/// Gets the singleton instance of the debug drawer
	/// </summary>
	static DebugDrawer& Get();
	/// <summary>
	/// Disposes of all resources used by the debug drawer
	/// </summary>
	static void Uninitialize();

	/// <summary>
	/// Pushes a new color to the stack, replacing the existing value
	/// Will be used by commands that do not specify a color
	/// </summary>
	/// <param name="color">The new color for elements</param>
	void PushColor(const glm::vec3& color);
	/// <summary>
	/// Pops a color from the stack, replacing the existing value
	/// Will be used by commands that do not specify a color
	/// </summary>
	glm::vec3 PopColor();

	/// <summary>
	/// Pushes a new transform to the stack, replacing the existing value and flushing all lines and tris
	/// </summary>
	/// <param name="world">The new world transform to use for drawing</param>
	void PushWorldMatrix(const glm::mat4& world);
	/// <summary>
	/// Pops a transform from the stack, replacing the existing value and flushing all lines and tris
	/// </summary>
	void PopWorldMatrix();

	/// <summary>
	/// Draws a line between 2 points using the current debug color
	/// </summary>
	/// <param name="p1">The first point</param>
	/// <param name="p2">The second point</param>
	void DrawLine(const glm::vec3& p1, const glm::vec3& p2);
	/// <summary>
	/// Draws a line between 2 points using a given color
	/// </summary>
	/// <param name="p1">The first point</param>
	/// <param name="p2">The second point</param>
	/// <param name="color">Color for line</param>
	void DrawLine(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& color = glm::vec3(1.0f));
	/// <summary>
	/// Draws a line between 2 points using 2 different colors
	/// </summary>
	/// <param name="p1">The first point</param>
	/// <param name="p2">The second point</param>
	/// <param name="c1">Color for first point</param>
	/// <param name="c2">Color for second point</param>
	void DrawLine(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& color1, const glm::vec3& color2);
	/// <summary>
	/// Flushes all lines to the screen, resetting our line count to 0s
	/// </summary>
	void FlushLines();

	/// <summary>
	/// Draws a triangle between 3 points using the current debug color
	/// Remember to keep winding order in mind!
	/// </summary>
	/// <param name="p1">The first point</param>
	/// <param name="p2">The second point</param>
	/// <param name="p3">The third point</param>
	void DrawTri(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3);
	/// <summary>
	/// Draws a triangle between 3 points with a given color
	/// Remember to keep winding order in mind!
	/// </summary>
	/// <param name="p1">The first point</param>
	/// <param name="p2">The second point</param>
	/// <param name="p3">The third point</param>
	/// <param name="color">Color for triangle</param>
	void DrawTri(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& color = glm::vec3(1.0f));
	/// <summary>
	/// Draws a triangle between 3 points with 3 unique colors
	/// Remember to keep winding order in mind!
	/// </summary>
	/// <param name="p1">The first point</param>
	/// <param name="p2">The second point</param>
	/// <param name="p3">The third point</param>
	/// <param name="c1">Color for first point</param>
	/// <param name="c2">Color for second point</param>
	/// <param name="c3">Color for third point</param>
	void DrawTri(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& c1, const glm::vec3& c2, const glm::vec3& c3);
	/// <summary>
	/// Flushes all triangles to the screen, resetting our triangle count to 0s
	/// </summary>
	void FlushTris();

	/// <summary>
	/// Flushes any remaining triangles and lines, drawing them to the screen and resetting their counters
	/// </summary>
	void FlushAll();

	/// <summary>
	/// Set the view projection matrix used by this debug drawer
	/// </summary>
	void SetViewProjection(const glm::mat4& viewProjection);

protected:
	DebugDrawer();

	std::stack<glm::vec3> _colorStack;
	std::stack<glm::mat4> _transformStack;
	glm::mat4    _viewProjection;
	glm::mat4    _worldMatrix;

	size_t       _lineOffset;
	VertexPosCol _lineBuffer[LINE_BATCH_SIZE * 2];
	size_t       _triangleOffset;
	VertexPosCol _triBuffer[TRI_BATCH_SIZE * 3];

	VertexBuffer::Sptr _linesVBO;
	VertexArrayObject::Sptr _linesVAO;
	VertexBuffer::Sptr _trisVBO;
	VertexArrayObject::Sptr _trisVAO;

	inline static DebugDrawer* __Instance = nullptr;
	inline static ShaderProgram::Sptr __Shader = nullptr;
};