#pragma once
#include "Gameplay/Physics/BulletDebugDraw.h"

#include <Logging.h>

#include "Graphics/DebugDraw.h"
#include "Utils/GlmBulletConversions.h"
#include "Utils/ImGuiHelper.h"

BulletDebugDraw::BulletDebugDraw() {
}

void BulletDebugDraw::drawLine(const btVector3& from, const btVector3& to, const btVector3& color) {
	DebugDrawer::Get().DrawLine(ToGlm(from), ToGlm(to), ToGlm(color));
}

void BulletDebugDraw::drawLine(const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor)
{
	DebugDrawer::Get().DrawLine(ToGlm(from), ToGlm(to), ToGlm(fromColor), ToGlm(toColor));
}

void BulletDebugDraw::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance,
									   int lifeTime, const btVector3& color) {

}

void BulletDebugDraw::reportErrorWarning(const char* warningString) {
	LOG_WARN(warningString);
}

void BulletDebugDraw::draw3dText(const btVector3& location, const char* textString) {

}

void BulletDebugDraw::setDebugMode(int debugMode)
{
	m_debugMode = debugMode;
}

bool BulletDebugDraw::DrawModeGui(const char* label, BulletDebugMode& mode)
{
	bool result = false;
	ImGui::TextUnformatted(label); 
	ImGui::SameLine();
	if (ImGui::BeginCombo("", (~mode).empty() ? "multiple" : (~mode).c_str())) {

		for (int ix = 0; ix <= 15; ix++) {
			bool selected = (int)mode & (1 << ix);
			BulletDebugMode itemMode = (BulletDebugMode)(1 << ix);
			std::string name = ~itemMode;
			if (ImGui::Checkbox(name.c_str(), &selected)) {
				mode = (BulletDebugMode)(*mode & ~*itemMode);
				if (selected) {
					mode = (BulletDebugMode)(*mode | *itemMode);
				}
				result = true;
			}
		}

		ImGui::EndCombo();
	}
	return result;
}
