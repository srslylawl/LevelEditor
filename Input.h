#pragma once
#include <functional>
#include <map>
#include <utility>
#include <SDL_keycode.h>

using namespace std;


class InputActionBinding {

public:
	string Name;
};

/**
 * \brief Handles Input and delegates it to subscribers
 */
class Input {
	Input() = default;
	inline static map<SDL_Keycode, vector<InputActionBinding>> Bindings;
public:
	void TestAdd(const SDL_Keycode keycode, string name) {
		const InputActionBinding binding{move(name)};
		Bindings[keycode].push_back(binding);
	}
};


