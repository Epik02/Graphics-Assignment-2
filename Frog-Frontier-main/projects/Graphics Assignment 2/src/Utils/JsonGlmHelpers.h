#pragma once
#include <GLM/glm.hpp>
#include "json.hpp"
#include "GLM/gtc/quaternion.hpp"

namespace glm {
	template <typename T, enum glm::qualifier Q = glm::qualifier::packed_highp>
	inline void to_json(nlohmann::json& j, const vec<2, T, Q>& value) {
		j = {
			{"x", value.x},
			{"y", value.y}
		};
	}
	template <typename T, enum glm::qualifier Q = glm::qualifier::packed_highp>
	inline void to_json(nlohmann::json& j, const vec<3, T, Q>& value) {
		j = {
			{"x", value.x},
			{"y", value.y},
			{"z", value.z}
		};
	}
	template <typename T, enum glm::qualifier Q = glm::qualifier::packed_highp>
	inline void to_json(nlohmann::json& j, const vec<4, T, Q>& value) {
		j =  {
			{"x", value.x},
			{"y", value.y},
			{"z", value.z},
			{"w", value.w}
		};
	}

	template <typename T, enum glm::qualifier Q = glm::qualifier::packed_highp>
	inline void to_json(nlohmann::json& j, const tquat<T, Q>& value) {
		j = {
			{"x", value.x},
			{"y", value.y},
			{"z", value.z},
			{"w", value.w}
		};
	}

	template <int R, int C, typename T, enum glm::qualifier Q = glm::qualifier::packed_highp>
	void to_json(nlohmann::json& j, const mat<R, C, T, Q>& mat) {
		nlohmann::json result = std::vector<nlohmann::json>();
		for (int ix = 0; ix < R; ix++) {
			result[ix] = mat[ix];
		}
		j = result;
	}

	template <typename T, enum glm::qualifier Q = glm::qualifier::packed_highp>
	void from_json(const nlohmann::json& j, vec<2, T, Q>& value) {
		j.at("x").get_to(value.x);
		j.at("y").get_to(value.y);
	}
	template <typename T, enum glm::qualifier Q = glm::qualifier::packed_highp>
	void from_json(const nlohmann::json& j, vec<3, T, Q>& value) {
		j.at("x").get_to(value.x);
		j.at("y").get_to(value.y);
		j.at("z").get_to(value.z);
	}
	template <typename T, enum glm::qualifier Q = glm::qualifier::packed_highp>
	void from_json(const nlohmann::json& j, vec<4, T, Q>& value) {
		j.at("x").get_to(value.x);
		j.at("y").get_to(value.y);
		j.at("z").get_to(value.z);
		j.at("w").get_to(value.w);
	}

	template <typename T, enum glm::qualifier Q = glm::qualifier::packed_highp>
	void from_json(const nlohmann::json& j, tquat<T, Q>& value) {
		j.at("x").get_to(value.x);
		j.at("y").get_to(value.y);
		j.at("z").get_to(value.z);
		j.at("w").get_to(value.w);
	}

	template <int R, int C, typename T, enum glm::qualifier Q = glm::qualifier::packed_highp>
	void from_json(const nlohmann::json& j, mat<R, C, T, Q>& value) {
	for (int ix = 0; ix < R; ix++) {
		j.at(ix).get_to(value[ix]);
	}
}
}

template <typename T>
T JsonGet(const nlohmann::json& blob, const std::string& key, const T& defaultRet = T()) {
	if (blob.find(key) != blob.end()) {
		return blob[key].get<T>();
	} else {
		return defaultRet;
	}
}

#define JsonParseEnum(typeName, blob, key, defaultVal) (blob.contains(key) ? Parse##typeName(blob[key].get<std::string>(), defaultVal) : defaultVal)