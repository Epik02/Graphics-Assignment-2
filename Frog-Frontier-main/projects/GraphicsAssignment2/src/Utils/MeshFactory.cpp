#include "Utils/MeshFactory.h"

MeshBuilderParam MeshBuilderParam::CreateCube(const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& eulerDeg /*= glm::vec3(0.0f)*/, const glm::vec4& col /*= glm::vec4(1.0f)*/) {
	MeshBuilderParam result;
	result.Type = MeshBuilderType::Cube;
	result.Color = col;
	result.Params["position"] = pos;
	result.Params["scale"] = scale;
	result.Params["rotation"] = eulerDeg;
	return result;
}

MeshBuilderParam MeshBuilderParam::CreateIcoSphere(const glm::vec3& center, float radius, int tessellation /*= 0*/, const glm::vec4& col /*= glm::vec4(1.0f)*/) {
	return MeshBuilderParam::CreateIcoSphere(center, glm::vec3(radius), tessellation, col);
}

MeshBuilderParam MeshBuilderParam::CreateIcoSphere(const glm::vec3& center, const glm::vec3& radii, int tessellation /*= 0*/, const glm::vec4& col /*= glm::vec4(1.0f)*/) {
	MeshBuilderParam result;
	result.Type = MeshBuilderType::IcoShere;
	result.Color = col;
	result.Params["position"] = center;
	result.Params["radius"] = radii;
	result.Params["tessellation"] = glm::vec3(tessellation, 0.0f, 0.0f);
	return result;
}

MeshBuilderParam MeshBuilderParam::CreateUVSphere(const glm::vec3& center, float radius, int tessellation /*= 0*/, const glm::vec4& col /*= glm::vec4(1.0f)*/) {
	return MeshBuilderParam::CreateUVSphere(center, glm::vec3(radius), tessellation, col);
}

MeshBuilderParam MeshBuilderParam::CreateUVSphere(const glm::vec3& center, const glm::vec3& radii, int tessellation /*= 0*/, const glm::vec4& col /*= glm::vec4(1.0f)*/) {
	MeshBuilderParam result;
	result.Type = MeshBuilderType::UvSphere;
	result.Color = col;
	result.Params["position"] = center;
	result.Params["radius"] = radii;
	result.Params["tessellation"] = glm::vec3(tessellation, 0.0f, 0.0f);
	return result;
}

MeshBuilderParam MeshBuilderParam::CreatePlane(const glm::vec3& pos, const glm::vec3& normal, const glm::vec3& tangent, const glm::vec2& scale, const glm::vec2& uvScale, const glm::vec4& col /*= glm::vec4(1.0f)*/) {
	MeshBuilderParam result;
	result.Type = MeshBuilderType::Plane;
	result.Color = col;
	result.Params["position"] = pos;
	result.Params["normal"] = normal;
	result.Params["tangent"] = tangent;
	result.Params["scale"] = glm::vec3(scale, 0.0f);
	result.Params["uv_scale"] = glm::vec3(uvScale, 1.0f);
	return result;
}

MeshBuilderParam MeshBuilderParam::CreateInvert() {
	MeshBuilderParam result;
	result.Type = MeshBuilderType::FaceInvert;
	return result;
}

MeshBuilderParam MeshBuilderParam::FromJson(const nlohmann::json& blob) {
	MeshBuilderParam result;
	result.Type = JsonParseEnum(MeshBuilderType, blob, "type", MeshBuilderType::Unknown);
	LOG_ASSERT(result.Type != MeshBuilderType::Unknown, "Failed to get type!");
	result.Color = blob["color"];
	if (blob.contains("params") && blob["params"].is_object()) {
		for (auto& [key, value] : blob["params"].items()) {
			result.Params[key] = value;
		}
	}
	return result;
}

nlohmann::json MeshBuilderParam::ToJson() const {
	nlohmann::json result;
	result["type"] = ~Type;
	result["color"] = Color;
	result["params"] = nlohmann::json();
	for (auto& [key, value] : Params) {
		result["params"][key] = value;
	}
	return result;
}