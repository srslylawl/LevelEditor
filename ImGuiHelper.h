#pragma once
#include <imgui.h>

namespace ImGuiHelper {
	inline void Image(const int textureId, const ImVec2 size = ImVec2(32, 32)) {
		ImGui::Image(reinterpret_cast<void*>(textureId), size, ImVec2(0, 1), ImVec2(1, 0));
	}

	inline bool ImageButton(const int textureId, const ImVec2 size = ImVec2(32, 32)) {
		return ImGui::ImageButton(reinterpret_cast<void*>(textureId), size, ImVec2(0, 1), ImVec2(1, 0));
	}
}

