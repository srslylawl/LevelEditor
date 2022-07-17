#include "TilePattern.h"
#include "ImGuiHelper.h"
#include "Resources.h"
#include "Texture.h"




int hasFlag(const Tiles::SurroundingTileFlags& mask, const Tiles::SurroundingTileFlags& flag) {
	const int iFlag = static_cast<int>(flag);
	return (static_cast<int>(mask) & iFlag) == iFlag;
}


Tiles::TileSlot* Tiles::TilePattern::GetTileSlot(const SurroundingTileFlags& mask) {
	const auto pattern = PatternFromSurroundingTiles(mask);

	const auto it = TileSlots.find(pattern);
	if (it != TileSlots.end()) return &it->second;

	return nullptr;
}

void Tiles::TilePattern::DearImGuiEditPattern() {
	using namespace ImGui;
	const auto drawFileImageButton = [this](const PatternFlag& pattern, const char* description) {
		unsigned int texId = 0;
		if (auto it = TileSlots.find(pattern); it != TileSlots.end()) {
			auto tileSlot = it->second;
			Rendering::Texture* tex;
			if (!tileSlot.TileSprites.empty() && Resources::TryGetTexture(tileSlot.TileSprites[0].Texture.c_str(), tex)) {
				texId = tex->GetTextureID();
			}
		}
		ImGuiHelper::Image(texId);
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Texture")) {
				const Rendering::Texture* t = *static_cast<const Rendering::Texture**>(payload->Data);
				AddTextureVariant(pattern, t->GetRelativeFilePath());
			}
			ImGui::EndDragDropTarget();
		}
		if (IsItemHovered()) {
			BeginTooltip();
			Text(description);
			EndTooltip();
		}
	};

	// Show regular grid first
	drawFileImageButton(PatternFlag::OUTER_TOP_LEFT, "Top left");
	SameLine();
	drawFileImageButton(PatternFlag::OUTER_TOP_MID, "Top middle");
	SameLine();
	drawFileImageButton(PatternFlag::OUTER_TOP_RIGHT, "Top right");
	drawFileImageButton(PatternFlag::OUTER_MID_LEFT, "Middle left");
	SameLine();
	drawFileImageButton(PatternFlag::CENTER, "Center");
	SameLine();
	drawFileImageButton(PatternFlag::OUTER_MID_RIGHT, "Middle right");
	drawFileImageButton(PatternFlag::OUTER_BOT_LEFT, "Bottom left");
	SameLine();
	drawFileImageButton(PatternFlag::OUTER_BOT_MID, "Bottom middle");
	SameLine();
	drawFileImageButton(PatternFlag::OUTER_BOT_RIGHT, "Bottom right");
}


Tiles::PatternFlag Tiles::TilePattern::PatternFromSurroundingTiles(const SurroundingTileFlags& mask) {
	const int straightSum = hasFlag(mask, SurroundingTileFlags::LEFT) +
		hasFlag(mask, SurroundingTileFlags::RIGHT) +
		hasFlag(mask, SurroundingTileFlags::UP) +
		hasFlag(mask, SurroundingTileFlags::DOWN);

	const int diagonalSum = hasFlag(mask, SurroundingTileFlags::UP_LEFT) +
		hasFlag(mask, SurroundingTileFlags::UP_RIGHT) +
		hasFlag(mask, SurroundingTileFlags::DOWN_LEFT) +
		hasFlag(mask, SurroundingTileFlags::DOWN_RIGHT);

	//Singles/Horizontals
	if (straightSum + diagonalSum == 0) return PatternFlag::SOLO;

	if (straightSum == 1) {
		if (hasFlag(mask, SurroundingTileFlags::RIGHT)) return PatternFlag::SINGLE_LEFT;
		if (hasFlag(mask, SurroundingTileFlags::LEFT)) return PatternFlag::SINGLE_RIGHT;
		if (hasFlag(mask, SurroundingTileFlags::DOWN)) return PatternFlag::SINGLE_TOP;
		if (hasFlag(mask, SurroundingTileFlags::UP)) return PatternFlag::SINGLE_BOT;
	}

	if (straightSum == 2) {
		if (hasFlag(mask, SurroundingTileFlags::LEFT) && hasFlag(mask, SurroundingTileFlags::RIGHT)) return PatternFlag::HORIZONTAL_MIDDLE;
		if (hasFlag(mask, SurroundingTileFlags::UP) && hasFlag(mask, SurroundingTileFlags::DOWN)) return PatternFlag::VERTICAL_MIDDLE;
	}

	//TOP LEFT
	if (hasFlag(mask, SurroundingTileFlags::RIGHT) && hasFlag(mask, SurroundingTileFlags::DOWN)
		&& !hasFlag(mask, SurroundingTileFlags::UP) && !hasFlag(mask, SurroundingTileFlags::LEFT)) {
		return PatternFlag::OUTER_TOP_LEFT;
	}

	//TOP RIGHT
	if (!hasFlag(mask, SurroundingTileFlags::RIGHT) && hasFlag(mask, SurroundingTileFlags::DOWN)
		&& !hasFlag(mask, SurroundingTileFlags::UP) && hasFlag(mask, SurroundingTileFlags::LEFT)) {
		return PatternFlag::OUTER_TOP_RIGHT;
	}
	//RIGHT_CENTER
	if (!hasFlag(mask, SurroundingTileFlags::RIGHT) && hasFlag(mask, SurroundingTileFlags::DOWN)
		&& hasFlag(mask, SurroundingTileFlags::UP) && hasFlag(mask, SurroundingTileFlags::LEFT)) {
		return PatternFlag::OUTER_MID_RIGHT;
	}
	//LEFT_CENTER
	if (hasFlag(mask, SurroundingTileFlags::RIGHT) && hasFlag(mask, SurroundingTileFlags::DOWN)
		&& hasFlag(mask, SurroundingTileFlags::UP) && !hasFlag(mask, SurroundingTileFlags::LEFT)) {
		return PatternFlag::OUTER_MID_LEFT;
	}
	//TOP_CENTER
	if (hasFlag(mask, SurroundingTileFlags::RIGHT) && hasFlag(mask, SurroundingTileFlags::DOWN)
		&& !hasFlag(mask, SurroundingTileFlags::UP) && hasFlag(mask, SurroundingTileFlags::LEFT)) {
		return PatternFlag::OUTER_TOP_MID;
	}
	//BOTTOM_LEFT
	if (hasFlag(mask, SurroundingTileFlags::RIGHT) && !hasFlag(mask, SurroundingTileFlags::DOWN)
		&& hasFlag(mask, SurroundingTileFlags::UP) && !hasFlag(mask, SurroundingTileFlags::LEFT)) {
		return PatternFlag::OUTER_BOT_LEFT;
	}
	//BOTTOM_RIGHT
	if (!hasFlag(mask, SurroundingTileFlags::RIGHT) && !hasFlag(mask, SurroundingTileFlags::DOWN)
		&& hasFlag(mask, SurroundingTileFlags::UP) && hasFlag(mask, SurroundingTileFlags::LEFT)) {
		return PatternFlag::OUTER_BOT_RIGHT;
	}
	//BOTTOM_CENTER
	if (hasFlag(mask, SurroundingTileFlags::RIGHT) && !hasFlag(mask, SurroundingTileFlags::DOWN)
		&& hasFlag(mask, SurroundingTileFlags::UP) && hasFlag(mask, SurroundingTileFlags::LEFT)) {
		return PatternFlag::OUTER_BOT_MID;
	}
	//BOTTOM_LEFT_INVERSE
	if (hasFlag(mask, SurroundingTileFlags::RIGHT) && hasFlag(mask, SurroundingTileFlags::UP)
		&& hasFlag(mask, SurroundingTileFlags::LEFT) && !hasFlag(mask, SurroundingTileFlags::UP_RIGHT) && diagonalSum >= 3) {
		return PatternFlag::INNER_BOT_LEFT;
	}
	//BOTTOM_RIGHT_INVERSE
	if (hasFlag(mask, SurroundingTileFlags::RIGHT) && hasFlag(mask, SurroundingTileFlags::UP)
		&& hasFlag(mask, SurroundingTileFlags::LEFT) && !hasFlag(mask, SurroundingTileFlags::UP_LEFT) && diagonalSum >= 3) {
		return PatternFlag::INNER_BOT_RIGHT;
	}
	//TOP_LEFT_INVERSE
	if (hasFlag(mask, SurroundingTileFlags::RIGHT) && hasFlag(mask, SurroundingTileFlags::UP)
		&& hasFlag(mask, SurroundingTileFlags::LEFT) &&
		!hasFlag(mask, SurroundingTileFlags::DOWN_RIGHT) && diagonalSum >= 3) {
		return PatternFlag::INNER_TOP_LEFT;
	}
	//TOP_RIGHT_INVERSE
	if (hasFlag(mask, SurroundingTileFlags::RIGHT) && hasFlag(mask, SurroundingTileFlags::UP)
		&& hasFlag(mask, SurroundingTileFlags::LEFT) &&
		!hasFlag(mask, SurroundingTileFlags::DOWN_LEFT) && diagonalSum >= 3) {
		return PatternFlag::INNER_TOP_RIGHT;
	}

	return PatternFlag::CENTER;
}
