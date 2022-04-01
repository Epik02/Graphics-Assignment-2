/**
 * NOTE: you MAY NOT use this file in your GDW game or graphics assignments
 * (at least for the fall semester)
 * 
 * You may use this implementation as a reference to implement your own version
 * using similar concepts, and that fit better with your game
 */
#pragma once
#include <fstream>

#include "Graphics/VertexArrayObject.h"
#include "Graphics/VertexTypes.h"

#include "Utils/MeshBuilder.h"

/// <summary>
/// An optimized OBJ loader that can convert an OBJ file to a binary representation
/// that we can load significantly faster
/// </summary>
class OptimizedObjLoader {
public:
	/// <summary>
	/// Loads a VAO from an OBJ file. On the first time this is called for an OBJ file, will convert the OBJ file 
	/// to a binary file and load that instead. On subsequent runs, the binary file will be loaded instead
	/// </summary>
	/// <param name="filename">The path to the .obj or .bin file to load</param>
	/// <returns>A VAO loaded from disk</returns>
	static VertexArrayObject::Sptr LoadFromFile(const std::string& filename);
	/// <summary>
	/// Manually converts an OBJ file into a binary mesh file
	/// </summary>
	/// <param name="inFile">The path to OBJ file to convert</param>
	/// <param name="outFile">The output path for the bin file, or empty to use the inFile path and replace the extension with .bin</param>
	static void ConvertToBinary(const std::string& inFile, const std::string& outFile = "");

	/// <summary>
	/// Saves a mesh builder of the given type to a binary file
	/// </summary>
	/// <typeparam name="VertexType"></typeparam>
	/// <param name="mesh"></param>
	/// <param name="outFilename"></param>
	template <typename VertexType>
	static void SaveBinaryFile(MeshBuilder<VertexType>& mesh, const std::string& outFilename);

protected:
	// Will be put at the start of the binary file, contains info about the contents of the file
	struct BinaryHeader {
		// A check value so we can ensure that we're loading in the right file type
		char      HeaderBytes[4] ={ 'B', 'O', 'B', 'J' };
		// The version code, we can use this to create different loaders if our format changes
		uint16_t  Version = 0;
		// The number of indices in the mesh
		uint32_t  NumIndices = 0;
		// The type of index to load
		IndexType IndicesType = IndexType::Unknown;
		// The number of vertices in the mesh
		uint32_t  NumVertices = 0;
		// The size of a single vertex structure
		uint16_t  VertexStride = 0;
		// The number of vertex attributes (basically how many VDECL entries there are)
		uint8_t   NumAttributes = 0;
	};

	OptimizedObjLoader() = default;
	~OptimizedObjLoader() = default;

	static MeshBuilder<VertexPosNormTexColTangents>* _LoadFromObjFile(const std::string& filename);
	static VertexArrayObject::Sptr _LoadFromBinFile(const std::string& filename);
};

template <typename VertexType>
void OptimizedObjLoader::SaveBinaryFile(MeshBuilder<VertexType>& mesh, const std::string& outFilename) {
	// Open the output file
	std::ofstream file(outFilename, std::ios::binary);
	if (!file) {
		throw std::runtime_error("Failed to open output file");
	}

	// Create the fixed size header for our output file
	BinaryHeader header  = BinaryHeader();
	header.Version       = 0x01; // This is version 1! Update this and implement different readers if changes to format are made
	header.NumIndices    = mesh.GetIndexCount();
	header.IndicesType   = IndexType::UInt;
	header.NumVertices   = mesh.GetVertexCount();
	header.VertexStride  = sizeof(VertexType);
	header.NumAttributes = VertexType::V_DECL.size();

	// Write header bytes to the stream
	file.write(reinterpret_cast<const char*>(&header), sizeof(BinaryHeader));

	// Write which attributes we have to the stream
	for (int ix = 0; ix < VertexType::V_DECL.size(); ix++) {
		file.write(reinterpret_cast<const char*>(&VertexType::V_DECL[ix]), sizeof(BufferAttribute));
	}
	// Write any index data to the file
	if (mesh.GetIndexCount() > 0) {
		file.write(reinterpret_cast<const char*>(mesh.GetIndexDataPtr()), mesh.GetIndexCount() * sizeof(uint32_t));
	}

	// Write vertex data to file
	file.write(reinterpret_cast<const char*>(mesh.GetVertexDataPtr()), mesh.GetVertexCount() * sizeof(VertexType));
}
