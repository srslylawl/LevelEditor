#pragma once
#include <chrono>
#include <iostream>


using namespace std::chrono;


class Time {

	inline static bool init;
	inline static nanoseconds startTime;
	inline static nanoseconds lastTick;
	inline static double deltaTimeMS;
	inline static double deltaTime;
	Time() = default;

public:
	static void Init() {
		startTime = high_resolution_clock::now().time_since_epoch();
		lastTick = startTime;
		init = true;
	}

	static float GetTime() {
		if (!init) {
			std::cout << "GetTime called but Time not initiated!";
		}

		const auto epoch = high_resolution_clock::now().time_since_epoch();
		return duration_cast<duration<float, std::ratio<1>>>(epoch - startTime).count();
	}

	static void CalcDeltaTime() {
		if (!init) {
			std::cout << "CalcDeltaTime called but Time not initiated!" << std::endl;
		}
		const auto epoch = high_resolution_clock::now().time_since_epoch();

		auto deltaNano = epoch - lastTick;
		auto deltaNanoCount = deltaNano.count();
		deltaTimeMS = deltaNanoCount / 1000000.0;
		deltaTime = deltaTimeMS / 1000.0;
		//std::cout << "StartTime: " << startTime.count() << endl;
		//std::cout << "DeltaNano: " << deltaNano.count() << endl;
		//std::cout << "DeltaTimeMS: " << deltaTimeMS << endl;
		//std::cout << "DeltaTimeSeconds: " << deltaTime << endl;

		lastTick = epoch;
	}

	static double GetDeltaTime() { return deltaTime;  }
	static double GetDeltaTimeMS() { return deltaTimeMS; }
};

