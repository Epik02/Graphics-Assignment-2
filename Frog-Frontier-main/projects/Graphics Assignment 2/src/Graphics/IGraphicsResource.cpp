#include "Graphics/IGraphicsResource.h"

IGraphicsResource::IGraphicsResource() :
	_debugName(""),
	_rendererId(0)
{ }

void IGraphicsResource::SetDebugName(const std::string& name)
{
	_debugName = name;
	GlResourceType type = GetResourceClass();
	if (type != GlResourceType::Unknown && _rendererId != 0) {
		glObjectLabel(*type, _rendererId, name.size(), name.c_str());
	}
}

const std::string& IGraphicsResource::GetDebugName() const {
	return _debugName;
}

uint32_t IGraphicsResource::GetHandle() const {
	return _rendererId;
}

void IGraphicsResource::_SetRenderId(uint32_t renderId)
{
	_rendererId = renderId;

	GlResourceType type = GetResourceClass();
	if (type != GlResourceType::Unknown && _rendererId != 0 && !_debugName.empty()) {
		glObjectLabel(*type, _rendererId, _debugName.size(), _debugName.c_str());
	}
}
