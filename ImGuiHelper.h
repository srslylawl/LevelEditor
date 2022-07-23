#pragma once
#include <imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <functional>
namespace Rendering {
	class Texture;
}

namespace ImGuiHelper {
	inline void Image(const int textureId, const ImVec2 size = ImVec2(32, 32), const char* id = 0) {
		if (id != 0) ImGui::PushID(id);
		ImGui::Image(reinterpret_cast<ImTextureID>(textureId), size, ImVec2(0, 1), ImVec2(1, 0));
		if (id != 0) ImGui::PopID();
	}

	inline bool ImageButton(const int textureId, const ImVec2 size = ImVec2(32, 32), int framePadding = -1) {
		return ImGui::ImageButton(reinterpret_cast<ImTextureID>(textureId), size, ImVec2(0, 1), ImVec2(1, 0), framePadding);
	}

	inline void DropTargetTexture(std::function<void(Rendering::Texture*)> onDrop) {
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Texture")) {
				Rendering::Texture* t = *static_cast<Rendering::Texture**>(payload->Data);
				onDrop(t);
			}
			ImGui::EndDragDropTarget();
		}
	}

	inline void TextWithToolTip(const char* text, const char* tooltip) {
		ImGui::TextUnformatted(text);
		ImGui::SameLine();
		ImGui::TextDisabled("(?)");

		if(!ImGui::IsItemHovered()) return;

		ImGui::BeginTooltip();
		ImGui::TextUnformatted(tooltip);
		ImGui::EndTooltip();
	}
	void DragSourceTexture(Rendering::Texture*& texture);


	void TextCentered(const char* text);

}

