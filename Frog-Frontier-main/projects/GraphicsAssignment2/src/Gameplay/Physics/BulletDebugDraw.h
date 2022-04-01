#pragma once
#include "LinearMath/btIDebugDraw.h"
#include <EnumToString.h>

/// <summary>
/// Represents the options for debug drawing with bullet
/// </summary>
ENUM(BulletDebugMode, int,
	 None                 = 0,
	 DrawWireframe        = 1,
	 DrawAabb             = 2, //(1 << 1),
	 DrawFeaturesText     = 4, //(1 << 2),
	 DrawContactPoints    = 8, //(1 << 3),
	 NoDeactivation       = 16, //(1 << 4),
	 NoHelpText           = 32, //(1 << 5),
	 DrawText             = 64, //(1 << 6),
	 ProfileTimings       = 128, //(1 << 7),
	 EnableSatComparison  = 256, //(1 << 8),
	 DisableBulletLCP     = 512, //(1 << 9),
	 EnableCCD            = 1024, //(1 << 10),
	 DrawConstraints      = 2048, //(1 << 11),
	 DrawConstraintLimits = 4096, //(1 << 12),
	 FastWireframe        = 8192, //(1 << 13),
	 DrawNormals          = 16384, //(1 << 14),
	 DrawFrames           = 32768, //(1 << 15),
);

/// <summary>
/// Implements the btIDebugDraw interface, allowing us to draw physics debug UI
/// for our scenes
/// </summary>
class BulletDebugDraw : public btIDebugDraw
{
private:
	int m_debugMode;

public:
	BulletDebugDraw();

	// Inherited from btIDebugDraw

	virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
	virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor);
	virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color);
	virtual void reportErrorWarning(const char* warningString);
	virtual void draw3dText(const btVector3& location, const char* textString);
	virtual void setDebugMode(int debugMode);
	virtual int getDebugMode() const { return m_debugMode; }

	static bool DrawModeGui(const char* label, BulletDebugMode& mode);
};