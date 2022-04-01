#pragma once
#include "../ApplicationLayer.h"

class LogicUpdateLayer final : public ApplicationLayer {
public:
	MAKE_PTRS(LogicUpdateLayer)

	LogicUpdateLayer();
	virtual ~LogicUpdateLayer();

	// Inherited from ApplicationLayer

	virtual void OnUpdate() override;

protected:

};