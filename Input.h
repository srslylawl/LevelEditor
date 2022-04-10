#pragma once

//#define _HAS_STD_BYTE 0 //required to use windows 10 sdk, specifically GUIDs - wtf?

#include <combaseapi.h>
#include <functional>
#include <map>
#include <SDL_keycode.h>
#include <unordered_set>
#include <iostream>

struct InputActionBinding {
	std::function<void()> Action;
	GUID BindingGUID;
	SDL_Keycode Keycode;
	InputActionBinding(const GUID guid, std::function<void()> action, SDL_Keycode keycode) : Action(std::move(action)), BindingGUID(guid), Keycode(keycode) { }

	bool operator==(const InputActionBinding& t) const {
		return (this->BindingGUID == t.BindingGUID);
	}

	struct HashFunction {
		size_t operator()(const InputActionBinding& binding) const {
			RPC_STATUS status = RPC_S_OK;
			return ::UuidHash(&const_cast<GUID&>(binding.BindingGUID), &status);
		}
	};

};

/**
 * \brief Handles Input and delegates it to subscribers
 */
class Input {
	Input() = default;
	inline static std::map<SDL_Keycode, std::unordered_set<InputActionBinding, InputActionBinding::HashFunction>> Bindings;
public:
	static InputActionBinding AddBinding(const SDL_Keycode keycode, std::function<void()> action) {
		GUID newGUID;
		CoCreateGuid(&newGUID);
		InputActionBinding newBinding(newGUID, action, keycode);
		Bindings[keycode].insert(newBinding);
		return newBinding;
	}
	static void RemoveBinding(InputActionBinding* binding) {
		if(!Bindings[binding->Keycode].erase(*binding)) {
			std::cout << "Failed to remove binding for keycode: " << binding->Keycode << std::endl;
		}
	}

	static void ReceiveInput(const SDL_Keycode keycode) {
		for (auto binding : Bindings[keycode]) {
			binding.Action();
		}
	}
};




