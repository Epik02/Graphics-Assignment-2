#pragma once

/**
 * The timing class is a very simple singleton class that will store our timing values 
 * per-frame
 */
class Timing final {
public:
	Timing() = default;
	~Timing() = default;

	inline float DeltaTime() { return _deltaTime; }
	inline float UnscaledDeltaTime() { return _unscaledDeltaTime; }
	inline float TimeSinceSceneLoad() { return _timeSinceSceneLoad; }
	inline float UnscaledTimeSinceSceneLoad() { return _unscaledTimeSinceSceneLoad; }
	inline float TimeSinceAppLoad() { return _timeSinceSceneLoad; }
	inline float UnscaledTimeSinceAppLoad() { return _unscaledTimeSinceSceneLoad; }

	static inline Timing& Current() { return _singleton; }

	static inline float TimeScale() { return _timeScale; }
	static inline void SetTimeScale(float value) { _timeScale = value < 0.0f ? 0.0f : value; }

protected:
	friend class Application;

	static Timing _singleton;

	float _deltaTime = 0;
	float _unscaledDeltaTime = 0;
	float _timeSinceSceneLoad = 0;
	float _unscaledTimeSinceSceneLoad = 0;
	float _timeSinceAppLoad = 0;
	float _unscaledTimeSinceAppLoad = 0;

	static inline float _timeScale = 1.0f;
};

inline Timing Timing::_singleton = Timing();