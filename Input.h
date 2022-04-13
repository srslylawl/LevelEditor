#pragma once

//#define _HAS_STD_BYTE 0 //required to use windows 10 sdk, specifically GUIDs - wtf?

#include <combaseapi.h>
#include <functional>
#include <map>
#include <SDL_keycode.h>
#include <unordered_set>
#include <iostream>
#include <set>
#include <utility>

enum class KeyEvent {
	KeyDown = 1 << 1,
	KeyHold = 1 << 2,
	KeyUp = 1 << 3,
};

struct InputActionBinding {
	std::function<void(KeyEvent)> Action;
	GUID BindingGUID;
	SDL_Keycode Keycode;
	InputActionBinding(const GUID guid, std::function<void(KeyEvent)> action, SDL_Keycode keycode) : Action(std::move(action)), BindingGUID(guid), Keycode(keycode) { }
};

struct MouseMovementEvent {
	MouseMovementEvent(int mouse_pos_x, int mouse_pos_y, int mouse_delta_x, int mouse_delta_y,
		const std::map<Uint8, bool>* mouse_buttons_held_down)
		: MousePos_X(mouse_pos_x),
		  MousePos_Y(mouse_pos_y),
		  MouseDelta_X(mouse_delta_x),
		  MouseDelta_Y(mouse_delta_y),
		  mouseButtonsHeldDown(mouse_buttons_held_down) {}

	const int MousePos_X;
	const int MousePos_Y;
	const int MouseDelta_X;
	const int MouseDelta_Y;
	const std::map<Uint8, bool>* mouseButtonsHeldDown;

};

struct InputMouseBinding {
	std::function<void(const MouseMovementEvent*)> Action;
	GUID BindingGUID;

	InputMouseBinding(std::function<void(const MouseMovementEvent*)> action, GUID guid) : Action(std::move(action)), BindingGUID(guid) {}
};



/**
 * \brief Handles Input and delegates it to subscribers
 */
class Input {
	Input() = default;
	inline static std::map<SDL_Keycode, std::unordered_set<InputActionBinding*>> keyBindings;
	inline static std::unordered_set<InputMouseBinding*> mouseBindings;
	inline static std::map<SDL_Keycode, bool> keysHeldDown;
	inline static std::set<SDL_Keycode> keysPressedThisFrame;
	inline static std::map<Uint8, bool> mouseButtonsHeldDown;
public:
	static InputActionBinding* AddKeyBinding(const SDL_Keycode keycode, std::function<void(KeyEvent)> action) {
		GUID newGUID;
		CoCreateGuid(&newGUID);
		auto* newBinding = new InputActionBinding(newGUID, action, keycode);
		keyBindings[keycode].insert(newBinding);
		return newBinding;
	}
	static void RemoveKeyBinding(InputActionBinding* binding) {
		if(!keyBindings[binding->Keycode].erase(binding)) {
			std::cout << "Failed to remove binding for keycode: " << binding->Keycode << std::endl;
		}
		delete binding;
	}

	static void ReceiveKeyDownInput(const SDL_Keycode& keycode) {
		if (keysHeldDown[keycode]) return; //don't care if its already held down

		keysHeldDown[keycode] = true;

		keysPressedThisFrame.insert(keycode);

		for (auto const& binding : keyBindings[keycode]) {
			binding->Action(KeyEvent::KeyDown);
		}
	}

	static void ReceiveKeyUpInput(const SDL_Keycode& keycode) {
		if(!keysHeldDown[keycode]) {
			std::cout << "Key " << keycode << " release received, but was not held down?" << std::endl;
			return;
		}
		keysHeldDown.erase(keycode);

		for (auto const& binding : keyBindings[keycode]) {
			binding->Action(KeyEvent::KeyUp);
		}
	}

	static InputMouseBinding* AddMouseMovementBinding(const std::function<void(const MouseMovementEvent*)> &action) {
		GUID newGuid;
		CoCreateGuid(&newGuid);
		const auto binding = new InputMouseBinding(action, newGuid);

		mouseBindings.insert(binding);

		return binding;
	}

	static void RemoveMouseBinding(InputMouseBinding* binding) {
		if(!mouseBindings.erase(binding)) {
			std::cout << "Failed to remove mouseBinding" << std::endl;
		}

		delete binding;
	}

	static void ReceiveMouseMotion(const SDL_MouseMotionEvent& e) {
		const MouseMovementEvent mouseEvent = MouseMovementEvent(e.x, e.y, e.xrel, e.yrel, &mouseButtonsHeldDown);

		for (const auto& mouse_binding : mouseBindings) {
			mouse_binding->Action(&mouseEvent);
		}
	}

	static void ReceiveMouseUp(const SDL_MouseButtonEvent e) {
		if (!mouseButtonsHeldDown[e.button]) {
			return;
		}
		mouseButtonsHeldDown.erase(e.button);
	}

	static void ReceiveMouseDown(const SDL_MouseButtonEvent e) {
		if(mouseButtonsHeldDown[e.button]) {
			return;
		}
		mouseButtonsHeldDown[e.button] = true;
	}

	static void DelegateHeldButtons() {
		for (auto const& [keycode, beingHeld] : keysHeldDown) {
			for (auto const &binding : keyBindings[keycode] ) {
				binding->Action(KeyEvent::KeyHold);
			}
		}

		keysPressedThisFrame.clear();
	}

	static void Cleanup() {
		for (auto& [key, value] : keyBindings) {
			Cleanup(value);
		}

		Cleanup(mouseBindings);
	}

	template<typename T>
	static void Cleanup(T collection) {
		if (collection.size() > 0) {
			std::cout << "Input wasn't properly unsubscribed - remaining entries: " << collection.size() << std::endl;

			for (auto ptr : collection) {
				delete ptr;
			}
		}
	}
};




