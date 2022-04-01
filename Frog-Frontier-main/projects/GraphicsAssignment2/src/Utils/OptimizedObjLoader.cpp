#include "Utils/OptimizedObjLoader.h"

#include "ObjLoader.h"

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <filesystem>

#include "Utils/StringUtils.h"
#include "GLFW/glfw3.h"
#include "Logging.h"

const char HEADER_BYTES[4] = { 'B', 'O', 'B', 'J' };
const std::string binaryExtension = ".bin";

namespace fs = std::filesystem;

VertexArrayObject::Sptr OptimizedObjLoader::LoadFromFile(const std::string& filename) {
	// Get the file extension and lowercase it
	fs::path filePath = std::filesystem::path(filename);
	std::string extension = filePath.extension().string();
	StringTools::ToLower(extension);

	// Load regular 'ol OBJ files
	if (extension == ".obj") {
		// Get the binary path
		fs::path binPath = filePath.replace_extension(binaryExtension);
		// If the file does not exist, convert the OBJ file to a binary file
		if (!fs::exists(binPath)) {
			ConvertToBinary(filename, binPath.string());
		}
		// Load the corresponding binary file
		return _LoadFromBinFile(binPath.string());
	} 
	// Load our fancy binary files
	else if (extension == ".bin") {
		return _LoadFromBinFile(filename);
	}
	// We've never met this extension in our life
	else {
		LOG_WARN("Cannot load model from \"{}\"", filename);
		return nullptr;
	}
}

void OptimizedObjLoader::ConvertToBinary(const std::string& inFile, const std::string& outFile) {
	// Load in the input file
	MeshBuilder<VertexPosNormTexColTangents>* mesh = _LoadFromObjFile(inFile);

	float startTime = static_cast<float>(glfwGetTime());

	// If we didn't get an output path, just take the input and replace the extension
	std::string outFileName = outFile;
	if (outFileName.empty()) { 
		// Copy input path
		auto path = std::filesystem::path(inFile);
		// Change extension
		path.replace_extension(binaryExtension);
		// Stringify path
		outFileName = path.string();
	}

	// Save the mesh to the file
	SaveBinaryFile(*mesh, outFileName);

	float endTime = static_cast<float>(glfwGetTime());
	LOG_TRACE("Converted OBJ file to binary \"{}\" in {} seconds ({} vertices, {} indices)", inFile, endTime - startTime, mesh->GetVertexCount(), mesh->GetIndexCount());

	// We no longer need the mesh data, free it
	delete mesh;
}

MeshBuilder<VertexPosNormTexColTangents>* OptimizedObjLoader::_LoadFromObjFile(const std::string& filename) {
	// Open our file in binary mode
	std::ifstream file;
	file.open(filename, std::ios::binary);

	// If our file fails to open, we will throw an error
	if (!file) {
		throw std::runtime_error("Failed to open file");
	}

	// Could also take this in as a parameter
	glm::vec4 color = glm::vec4(1.0f);

	// Our attributes
	std::vector<glm::vec3>  positions;
	std::vector<glm::vec3>  normals;
	std::vector<glm::vec2>  uvs;
	std::vector<glm::ivec3> vertices;
	std::vector<uint32_t>   indices;

	// Maps a key generated from obj indices to a vertex index that
	// has been added to the mesh already
	std::unordered_map<uint64_t, uint32_t> vertexMap;

	// We'll use the mesh builder since it supports easily adding
	// vertices and indices
	MeshBuilder<VertexPosNormTexColTangents>* mesh = new MeshBuilder<VertexPosNormTexColTangents>();

	// Storage for temporary data
	std::string line;
	glm::vec3 vecData;
	glm::ivec3 vertexIndices;

	float startTime = static_cast<float>(glfwGetTime());

	// Read and process the entire file
	while (file.peek() != EOF) {
		// Read in the first part of the line (ex: f, v, vn, etc...)
		std::string command;
		file >> command;

		// We will ignore the rest of the line for comment lines
		if (command == "#") {
			std::getline(file, line);
		}

		// The v command defines a vertex's position
		else if (command == "v") {
			// Read in and store a position
			file >> vecData.x >> vecData.y >> vecData.z;
			positions.push_back(vecData);
		}
		// TODO: handle normals and textures
		else if (command == "vn") {
			// Read in and store a position
			file >> vecData.x >> vecData.y >> vecData.z;
			normals.push_back(vecData);
		} else if (command == "vt") {
			// Read in and store a position
			file >> vecData.x >> vecData.y;
			uvs.push_back(vecData);
		}

		// The f command defines a polygon in the mesh
		// NOTE: make sure you triangulate in blender, otherwise it will
		// output quads instead of triangles
		else if (command == "f") {
			// Read the rest of the line from the file
			std::getline(file, line);
			// Trim whitespace from either end of the line
			StringTools::Trim(line);
			// Create a string stream so we can use streaming operators on it
			std::stringstream stream = std::stringstream(line);

			uint32_t edges[4];
			int ix = 0;
			// Iterate over up to 4 sets of attributes
			for (; ix < 4; ix++) {
				if (stream.peek() != EOF) {
					// Load in the faces, split up by slashes
					char tempChar;
					vertexIndices = glm::ivec3(0);
					stream >> vertexIndices.x >> tempChar >> vertexIndices.y >> tempChar >> vertexIndices.z;
					// The OBJ format can have negative values, which are a reference from the last added attributes
					if (vertexIndices.x < 0) { vertexIndices.x = positions.size() + 1 + vertexIndices.x; }
					if (vertexIndices.y < 0) { vertexIndices.y = uvs.size()       + 1 + vertexIndices.y; }
					if (vertexIndices.z < 0) { vertexIndices.z = normals.size()   + 1 + vertexIndices.z; }

					// We can construct a key using a bitmask of the attribute indices
					// This let's us quickly look up a combination of attributes to see if it's already been added
					// Note that this limits us to 2,097,150 unique attributes for positions, normals and textures
					const uint64_t mask = 0b0'000000000000000000000'000000000000000000000'111111111111111111111;
					uint64_t key = ((vertexIndices.x & mask) << 42) | ((vertexIndices.y & mask) << 21) | (vertexIndices.z & mask);

					// Find the index associated with the combination of attributes
					auto it = vertexMap.find(key);

					// If it exists, we push the index to our indices
					if (it != vertexMap.end()) {
						edges[ix] = it->second;
					} else {
						vertices.push_back(vertexIndices - glm::ivec3(1));
						uint32_t index = static_cast<uint32_t>(vertices.size()) - 1;

						// Cache the index based on our key
						vertexMap[key] = index;
						// Add index to mesh, and add to edges list for if we are using quads
						edges[ix] = index;
					}
				}
				// We've reached the end of the line, break out of the loop
				else { break; }
			}

			// Handling for triangle faces
			if (ix == 3) {
				indices.push_back(edges[0]);
				indices.push_back(edges[1]);
				indices.push_back(edges[2]);
			}
			// Handling for quad faces
			else if (ix == 4) {
				indices.push_back(edges[0]);
				indices.push_back(edges[1]);
				indices.push_back(edges[2]);

				indices.push_back(edges[0]);
				indices.push_back(edges[2]);
				indices.push_back(edges[3]);
			}
		}
	}

	mesh->ReserveVertexSpace(vertices.size());
	for (const auto& vertexIndices : vertices) {
		// Construct a new vertex using the indices for the vertex
		VertexPosNormTexColTangents vertex;
		vertex.Position = positions[vertexIndices.x];
		vertex.UV       = vertexIndices.y != 0 ? uvs[vertexIndices.y] : glm::vec2(0.0f);
		vertex.Normal   = vertexIndices.z != 0 ? normals[vertexIndices.z] : glm::vec3(0.0f, 0.0f, 1.0f);
		vertex.Color    = color;

		// Add to the mesh, get index of the added vertex
		mesh->AddVertex(vertex);
	}
	mesh->ReserveIndexSpace(indices.size());
	for (uint32_t ix : indices) {
		mesh->AddIndex(ix);
	}

	// Calculate our tangents
	MeshFactory::CalculateTBN(*mesh);

	// Calculate and trace out how long it took us to load
	float endTime = static_cast<float>(glfwGetTime());
	LOG_TRACE("Loaded OBJ file \"{}\" in {} seconds ({} vertices, {} indices)", filename, endTime - startTime, mesh->GetVertexCount(), mesh->GetIndexCount());

	// Move our data into a VAO and return it
	return mesh;
}

VertexArrayObject::Sptr OptimizedObjLoader::_LoadFromBinFile(const std::string& filename) {

	// Open the output file
	std::ifstream file(filename, std::ios::binary);
	// If our file fails to open, we will throw an error
	if (!file) { throw std::runtime_error("Failed to open file"); }

	float startTime = static_cast<float>(glfwGetTime());

	// Get the file size so we can avoid reading past the end
	file.seekg(0, std::ios::end);
	size_t size = file.tellg();
	file.seekg(0, std::ios::beg);

	// Read the header from the file
	BinaryHeader header = BinaryHeader();
	if (size >= sizeof(BinaryHeader)) {
		file.read(reinterpret_cast<char*>(&header), sizeof(BinaryHeader));
	} else {
		LOG_ERROR("Not enough data in the file!");
		return nullptr;
	}

	// TODO: validate header

	// Handle our version
	if (header.Version == 0x01) {
		// Determine how many bytes we need in the file
		size_t requiredBytes =
			sizeof(BinaryHeader) +
			(header.NumAttributes * sizeof(BufferAttribute)) +
			(header.VertexStride * (size_t)header.NumVertices) +
			(header.NumIndices * GetIndexTypeSize(header.IndicesType));

		// Make sure there's enough data in the file
		if (size < requiredBytes) {
			LOG_ERROR("Not enough data in the file!");
			return nullptr;
		}

		// Read all attributes from the file, this is basically our VDECL
		std::vector<BufferAttribute> vertexDeclaration;
		vertexDeclaration.resize(header.NumAttributes);
		for (int ix = 0; ix < header.NumAttributes; ix++) {
			file.read(reinterpret_cast<char*>(&vertexDeclaration[ix]), sizeof(BufferAttribute));
		}

		// These will have the buffer pointers
		IndexBuffer::Sptr indices = nullptr;
		VertexBuffer::Sptr vertices = nullptr;

		// If we have index data, load it
		if (header.NumIndices > 0) {
			// Create index buffer
			indices = IndexBuffer::Create(BufferUsage::StaticDraw);

			// Create memory to store indices, then read from the file
			void* dataStore = malloc(header.NumIndices * GetIndexTypeSize(header.IndicesType));
			file.read(reinterpret_cast<char*>(dataStore), header.NumIndices * GetIndexTypeSize(header.IndicesType));
			
			// Load data into OpenGL and free the CPU memory we allocated
			indices->LoadData(dataStore, GetIndexTypeSize(header.IndicesType), header.NumIndices, header.IndicesType);
			free(dataStore);
		}

		// Create a new VBO
		vertices = VertexBuffer::Create(BufferUsage::StaticDraw);

		// Create memory to store vertices and load from file
		void* vertexStore = malloc(header.NumVertices * (size_t)header.VertexStride);
		file.read(reinterpret_cast<char*>(vertexStore), header.NumVertices * (size_t)header.VertexStride);

		// Load data into OpenGL and free the CPU copy
		vertices->LoadData(vertexStore, header.VertexStride, header.NumVertices);
		free(vertexStore);

		// Create the VAO and attach our index and vertex buffers
		VertexArrayObject::Sptr result = VertexArrayObject::Create();
		result->SetIndexBuffer(indices);
		result->AddVertexBuffer(vertices, vertexDeclaration);

		// Copy in the vertex declaration we loaded
		result->SetVDecl(vertexDeclaration);

		// Calculate and trace out how long it took us to load
		float endTime = static_cast<float>(glfwGetTime());
		LOG_TRACE("Loaded OBJ file \"{}\" in {} seconds ({} vertices, {} indices)", filename, endTime - startTime, header.NumVertices, header.NumIndices);

		return result;
	}

	return nullptr;
}
