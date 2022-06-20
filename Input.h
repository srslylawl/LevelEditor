#pragma once
#include <combaseapi.h>
#include <functional>
#include <map>
#include <SDL.h>
#include <unordered_set>
#include <set>
#include <utility>
#include <glm/vec2.hpp>


enum class KeyEvent {
	KeyDown = 1 << 0,
	KeyHold = 1 << 1,
	KeyUp = 1 << 2,
};

enum class MouseButton {
	Left = 1 << 0,
	Right = 1 << 1,
	Middle = 1 << 2,
	X1 = 1 << 3,
	X2 = 1 << 4
};

struct InputKeyBinding {
	std::function<void(KeyEvent)> Action;
	GUID BindingGUID;
	SDL_Keycode Keycode;
	InputKeyBinding(const GUID guid, std::function<void(KeyEvent)> action, SDL_Keycode keycode) : Action(std::move(action)), BindingGUID(guid), Keycode(keycode) { }
};

struct MouseMotion {
	int posX;
	int posY;
	int deltaX;
	int deltaY;
};

struct MouseWheel {
	Uint32 scrollDelta;
	float scrollDeltaPrecise;
};

class InputMouseEvent {
	const std::map<MouseButton, std::unordered_set<KeyEvent>>* keyEvents = nullptr;
	const MouseWheel* wheel;
public:
	const MouseMotion* motion;

	bool GetMouseKey(KeyEvent mouse_key_event, MouseButton button) const {
		const auto iterator = keyEvents->find(button);
		if (iterator == keyEvents->end()) return false;
		return iterator->second.find(mouse_key_event) != iterator->second.end();
	}

	bool GetMouseKeyDown(MouseButton button) const { return GetMouseKey(KeyEvent::KeyDown, button); }

	bool GetMouseKeyUp(MouseButton button) const { return GetMouseKey(KeyEvent::KeyUp, button); }

	bool GetMouseKeyHold(MouseButton button) const { return GetMouseKey(KeyEvent::KeyHold, button); }

	int GetMouseWheelDelta() const { return wheel->scrollDelta; }

	float GetMouseWheelDeltaPrecise() const { return wheel->scrollDeltaPrecise; }

	InputMouseEvent(const std::map<MouseButton, std::unordered_set<KeyEvent>>* key_events, const MouseMotion* motion, const MouseWheel* wheel)
		: keyEvents(key_events),
		wheel(wheel),
		motion(motion) {}
};


struct InputMouseBinding {
	std::function<void(const InputMouseEvent*)> Action;
	GUID BindingGUID;

	InputMouseBinding(std::function<void(const InputMouseEvent*)> action, GUID guid) : Action(std::move(action)), BindingGUID(guid) {}
};

class Input {
	Input();
	inline static std::map<SDL_Keycode, std::unordered_set<InputKeyBinding*>> keyBindings;
	inline static std::unordered_set<SDL_Keycode> keysDownThisFrame;
	inline static std::unordered_set<SDL_Keycode> keysUpThisFrame;
	inline static std::unordered_set<InputMouseBinding*> mouseBindings;
	inline static std::map<SDL_Keycode, bool> keysHeldDown;
	inline static std::map<MouseButton, std::unordered_set<KeyEvent>> mouseKeyEvents;
	inline static MouseMotion mouseMotion;
	inline static MouseWheel mouseWheel;
	inline static std::pair<int, int> savedMousePosition;
public:
	static InputKeyBinding* AddKeyBinding(const SDL_Keycode keycode, std::function<void(KeyEvent)> action);

	static void RemoveKeyBinding(InputKeyBinding* binding);

	static void ReceiveKeyDownInput(const SDL_Keycode& keycode);

	static void ReceiveKeyUpInput(const SDL_Keycode& keycode);

	static InputMouseBinding* AddMouseBinding(const std::function<void(const InputMouseEvent*)>& action);

	static void RemoveMouseBinding(InputMouseBinding* binding);

	static void ReceiveMouseMotion(const SDL_MouseMotionEvent& e);

	static void ReceiveMouseButtonEvent(const SDL_MouseButtonEvent e);

	static void ReceiveMouseWheelEvent(const SDL_MouseWheelEvent e);

	static void DelegateMouseActions();

	static void DelegateKeyboardActions();

	static void ClearMouseActions();

	static void ClearKeyboardActions();

	static void SetMouseCapture(bool set);

	static glm::vec2 GetMousePosition();
	static void GetMousePosition(int& x, int& y);


	static void Cleanup();

	template<typename T>
	static void Cleanup(T collection);
};




