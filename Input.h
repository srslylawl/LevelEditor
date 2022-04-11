#pragma once

//#define _HAS_STD_BYTE 0 //required to use windows 10 sdk, specifically GUIDs - wtf?

#include <combaseapi.h>
#include <functional>
#include <map>
#include <SDL_keycode.h>
#include <unordered_set>
#include <iostream>
#include <set>
#include <xlocmon>

enum class KeyEvent {
	KeyDown = 1,
	KeyHold = 2,
	KeyUp = 4,
};

struct InputActionBinding {
	std::function<void(KeyEvent)> Action;
	GUID BindingGUID;
	SDL_Keycode Keycode;
	InputActionBinding(const GUID guid, std::function<void(KeyEvent)> action, SDL_Keycode keycode) : Action(std::move(action)), BindingGUID(guid), Keycode(keycode) { }

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
	inline static std::map<SDL_Keycode, std::unordered_set<InputActionBinding, InputActionBinding::HashFunction>> bindings;
	inline static std::map<SDL_Keycode, bool> keysHeldDown;
	inline static std::set<SDL_Keycode> keysPressedThisFrame;
public:
	static InputActionBinding AddBinding(const SDL_Keycode keycode, std::function<void(KeyEvent)> action) {
		GUID newGUID;
		CoCreateGuid(&newGUID);
		InputActionBinding newBinding(newGUID, action, keycode);
		bindings[keycode].insert(newBinding);
		return newBinding;
	}
	static void RemoveBinding(InputActionBinding* binding) {
		if(!bindings[binding->Keycode].erase(*binding)) {
			std::cout << "Failed to remove binding for keycode: " << binding->Keycode << std::endl;
		}
	}

	static void ReceiveKeyDownInput(const SDL_Keycode& keycode) {
		if (keysHeldDown[keycode]) return; //don't care if its already held down

		keysHeldDown[keycode] = true;

		keysPressedThisFrame.insert(keycode);

		for (auto const& binding : bindings[keycode]) {
			binding.Action(KeyEvent::KeyDown);
		}
	}

	static void ReceiveKeyUpInput(const SDL_Keycode& keycode) {
		if(!keysHeldDown[keycode]) {
			std::cout << "Key " << keycode << " release received, but was not held down?" << std::endl;
			return;
		}
		keysHeldDown.erase(keycode);

		for (auto const& binding : bindings[keycode]) {
			binding.Action(KeyEvent::KeyUp);
		}
	}

	static void ReceiveMouseMotion(const SDL_MouseMotionEvent& e) {
		int x = e.x;
		int y = e.y;
		int xrel = e.xrel;
		int yrel = e.yrel;

		std::cout << "Mouse: X: " << x << ", Y: " << y << std::endl;
		std::cout << "MouseRel: XRel: " << xrel << ", YRel: " << yrel << std::endl;
	}

	static void ReceiveMouseUp(const SDL_MouseButtonEvent e) {

	}

	static void ReceiveMouseDown(const SDL_MouseButtonEvent e) {

	}

	static void DelegateHeldButtons() {
		for (auto const& [keycode, beingHeld] : keysHeldDown) {
			for (auto const &binding : bindings[keycode] ) {
				binding.Action(KeyEvent::KeyHold);
			}
		}

		keysPressedThisFrame.clear();
	}
};




