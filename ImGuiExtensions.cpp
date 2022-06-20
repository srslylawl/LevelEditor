#include "imgui.h"

namespace ImGui {
	void TextCentered(const char* text) {
		float win_width = ImGui::GetWindowSize().x;
		float text_width = ImGui::CalcTextSize(text).x;

		// calculate the indentation that centers the text on one line, relative
		// to window left, regardless of the `ImGuiStyleVar_WindowPadding` value
		float text_indentation = (win_width - text_width) * 0.5f;

		// if text is too long to be drawn on one line, `text_indentation` can
		// become too small or even negative, so we check a minimum indentation
		float min_indentation = 20.0f;
		if (text_indentation <= min_indentation) {
			text_indentation = min_indentation;
		}

		ImGui::SameLine(text_indentation);
		ImGui::PushTextWrapPos(win_width - text_indentation);
		ImGui::TextWrapped(text);
		ImGui::PopTextWrapPos();
	}
}
