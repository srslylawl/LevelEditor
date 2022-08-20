#include "TilePatterns.h"
#include "ImGuiHelper.h"
#include "Resources.h"
#include "Texture.h"


int hasFlag(const Tiles::SurroundingTileFlags& mask, const Tiles::SurroundingTileFlags& flag) {
	const int iFlag = static_cast<int>(flag);
	return (static_cast<int>(mask) & iFlag) == iFlag;
}


const Tiles::TileSlot* Tiles::AutoTilePattern::GetTileSlot(const SurroundingTileFlags& mask) const {
	const auto pattern = PatternFromSurroundingTiles(mask);

	const auto it = TileSlots.find(pattern);
	if (it != TileSlots.end()) return &it->second;

	return nullptr;
}

void DrawTileSlotButton(Tiles::TileSlot* tileSlot, const char* description, int pattern, std::function<void(Rendering::Texture*)> onDropTexture, ImVec2 size = ImVec2(32, 32)) {
	using namespace ImGui;
	unsigned int texId = 0;
	Rendering::Texture* tex = nullptr;

	if (tileSlot != nullptr) {
		if (!tileSlot->TileSprites.empty() && Resources::TryGetTexture(tileSlot->TileSprites.back().TextureId, tex)) {
			texId = tex->GetTextureID();
		}
	}
	std::string id = "ImageId" + std::to_string(pattern);
	ImGuiHelper::Image(texId, size, id.c_str());
	ImGuiHelper::DropTargetTexture(onDropTexture);
	ImGuiHelper::DragSourceTexture(tex);

	if (IsItemHovered()) {
		BeginTooltip();
		std::string descriptionString = description;
		if (tileSlot && tileSlot->TileSprites.size() > 1) descriptionString += " (+" + std::to_string(tileSlot->TileSprites.size() - 1) + " more textures.) \nRight-click to view.";
		TextUnformatted(descriptionString.c_str());
		EndTooltip();
	}
	if (!tileSlot || tileSlot->TileSprites.empty() || !BeginPopupContextItem(id.c_str())) return;
	//when right-clicked, display all variants in a popup, aligned in a square pattern
	const auto squareRoot = (int)sqrt(tileSlot->TileSprites.size());
	int index = 0;
	for (auto it = tileSlot->TileSprites.begin(); it != tileSlot->TileSprites.end();) {
		if (index++ % squareRoot != 0) ImGui::SameLine();
		unsigned int textureId = 0;
		Rendering::Texture* variantTex = nullptr;
		if (Resources::TryGetTexture(it->TextureId, variantTex))
			textureId = variantTex->GetTextureID();

		ImGuiHelper::Image(textureId);
		ImGuiHelper::DragSourceTexture(variantTex);
		bool removed = false;
		//when hovered, display tooltip and allow deletion with "X" key
		if (IsItemHovered()) {
			BeginTooltip();
			if (variantTex != nullptr) {
				TextUnformatted(variantTex->Name.c_str());
			}
			TextUnformatted("Press 'X' while hovering to remove");
			EndTooltip();
			if (IsKeyPressed(ImGuiKey_X, false)) {
				it = tileSlot->TileSprites.erase(it);
				removed = true;
			}
		}
		if (!removed) ++it;
	}
	EndPopup();
}

void Tiles::AutoTilePattern::RenderDearImGui() {
	using namespace ImGui;
	auto wrapDrawTileSlotButton = [this](const AutoTilePatternFlag& pattern, const char* description) {
		TileSlot* tileSlot = nullptr;
		if (auto it = this->TileSlots.find(pattern); it != TileSlots.end())
			tileSlot = &it->second;

		DrawTileSlotButton(tileSlot, description, (int)pattern,
						   [this, &pattern](Rendering::Texture* t) {
			AddTextureVariant(static_cast<int>(pattern), t->AssetId);
		});
	};
	// Show regular grid first
	wrapDrawTileSlotButton(AutoTilePatternFlag::OUTER_TOP_LEFT, "Top left");
	SameLine();
	wrapDrawTileSlotButton(AutoTilePatternFlag::OUTER_TOP_MID, "Top middle");
	SameLine();
	wrapDrawTileSlotButton(AutoTilePatternFlag::OUTER_TOP_RIGHT, "Top right");
	wrapDrawTileSlotButton(AutoTilePatternFlag::OUTER_MID_LEFT, "Middle left");
	SameLine();
	wrapDrawTileSlotButton(AutoTilePatternFlag::CENTER, "Center");
	SameLine();
	wrapDrawTileSlotButton(AutoTilePatternFlag::OUTER_MID_RIGHT, "Middle right");
	wrapDrawTileSlotButton(AutoTilePatternFlag::OUTER_BOT_LEFT, "Bottom left");
	SameLine();
	wrapDrawTileSlotButton(AutoTilePatternFlag::OUTER_BOT_MID, "Bottom middle");
	SameLine();
	wrapDrawTileSlotButton(AutoTilePatternFlag::OUTER_BOT_RIGHT, "Bottom right");

	Separator();

	// Inner corners
	wrapDrawTileSlotButton(AutoTilePatternFlag::INNER_TOP_LEFT, "Inner top left");
	SameLine();
	wrapDrawTileSlotButton(AutoTilePatternFlag::INNER_TOP_RIGHT, "Inner top right");
	wrapDrawTileSlotButton(AutoTilePatternFlag::INNER_BOT_LEFT, "Inner bottom left");
	SameLine();
	wrapDrawTileSlotButton(AutoTilePatternFlag::INNER_BOT_RIGHT, "Inner bottom right");
}

void Tiles::SimpleTilePattern::RenderDearImGui() {
	DrawTileSlotButton(&tileSlot, "Tile Texture", 0, [this](Rendering::Texture* t){
		AddTextureVariant(0, t->AssetId);
	});
}

void Tiles::AutoWallPattern::RenderDearImGui() {
	auto wrapDrawTileSlotButton = [this](const AutoWallPatternFlag& pattern, const char* description) {
		TileSlot* tileSlot = nullptr;
		if (auto it = this->TileSlots.find(pattern); it != TileSlots.end())
			tileSlot = &it->second;

		DrawTileSlotButton(tileSlot, description, (int)pattern,
						   [this, &pattern](Rendering::Texture* t) {
			AddTextureVariant(static_cast<int>(pattern), t->AssetId);
		}, ImVec2(32, 64));
	};

	//Single Wall
	wrapDrawTileSlotButton(AutoWallPatternFlag::SOLO, "Standalone Wall");

	//Other
	wrapDrawTileSlotButton(AutoWallPatternFlag::LEFT, "Left Wall");
	ImGui::SameLine();
	wrapDrawTileSlotButton(AutoWallPatternFlag::CENTER, "Center(Middle) Wall");
	ImGui::SameLine();
	wrapDrawTileSlotButton(AutoWallPatternFlag::RIGHT, "Right Wall");
}

Tiles::AutoWallPatternFlag Tiles::AutoWallPattern::PatternFromSurroundingTiles(const SurroundingTileFlags& mask) {
	const bool hasLeft = hasFlag(mask, SurroundingTileFlags::LEFT);
	const bool hasRight = hasFlag(mask, SurroundingTileFlags::RIGHT);

	if(hasLeft && hasRight) {
		return AutoWallPatternFlag::CENTER;
	}

	if(hasLeft && !hasRight) {
		return AutoWallPatternFlag::RIGHT;
	}

	if(hasRight && !hasLeft) {
		return AutoWallPatternFlag::LEFT;
	}

	return AutoWallPatternFlag::SOLO;
}

const Tiles::TileSlot* Tiles::AutoWallPattern::GetTileSlot(const SurroundingTileFlags& mask) const {

	const auto pattern = PatternFromSurroundingTiles(mask);

	const auto it = TileSlots.find(pattern);
	if (it != TileSlots.end()) return &it->second;

	return nullptr;
	
}


Tiles::AutoTilePatternFlag Tiles::AutoTilePattern::PatternFromSurroundingTiles(const SurroundingTileFlags& mask) {
	const int straightSum = hasFlag(mask, SurroundingTileFlags::LEFT) +
		hasFlag(mask, SurroundingTileFlags::RIGHT) +
		hasFlag(mask, SurroundingTileFlags::UP) +
		hasFlag(mask, SurroundingTileFlags::DOWN);

	const int diagonalSum = hasFlag(mask, SurroundingTileFlags::UP_LEFT) +
		hasFlag(mask, SurroundingTileFlags::UP_RIGHT) +
		hasFlag(mask, SurroundingTileFlags::DOWN_LEFT) +
		hasFlag(mask, SurroundingTileFlags::DOWN_RIGHT);

	//Singles/Horizontals
	if (straightSum + diagonalSum == 0) return AutoTilePatternFlag::SOLO;

	if (straightSum == 1) {
		if (hasFlag(mask, SurroundingTileFlags::RIGHT)) return AutoTilePatternFlag::SINGLE_LEFT;
		if (hasFlag(mask, SurroundingTileFlags::LEFT)) return AutoTilePatternFlag::SINGLE_RIGHT;
		if (hasFlag(mask, SurroundingTileFlags::DOWN)) return AutoTilePatternFlag::SINGLE_TOP;
		if (hasFlag(mask, SurroundingTileFlags::UP)) return AutoTilePatternFlag::SINGLE_BOT;
	}

	if (straightSum == 2) {
		if (hasFlag(mask, SurroundingTileFlags::LEFT) && hasFlag(mask, SurroundingTileFlags::RIGHT)) return AutoTilePatternFlag::HORIZONTAL_MIDDLE;
		if (hasFlag(mask, SurroundingTileFlags::UP) && hasFlag(mask, SurroundingTileFlags::DOWN)) return AutoTilePatternFlag::VERTICAL_MIDDLE;
	}

	//TOP LEFT
	if (hasFlag(mask, SurroundingTileFlags::RIGHT) && hasFlag(mask, SurroundingTileFlags::DOWN)
		&& !hasFlag(mask, SurroundingTileFlags::UP) && !hasFlag(mask, SurroundingTileFlags::LEFT)) {
		return AutoTilePatternFlag::OUTER_TOP_LEFT;
	}

	//TOP RIGHT
	if (!hasFlag(mask, SurroundingTileFlags::RIGHT) && hasFlag(mask, SurroundingTileFlags::DOWN)
		&& !hasFlag(mask, SurroundingTileFlags::UP) && hasFlag(mask, SurroundingTileFlags::LEFT)) {
		return AutoTilePatternFlag::OUTER_TOP_RIGHT;
	}
	//RIGHT_CENTER
	if (!hasFlag(mask, SurroundingTileFlags::RIGHT) && hasFlag(mask, SurroundingTileFlags::DOWN)
		&& hasFlag(mask, SurroundingTileFlags::UP) && hasFlag(mask, SurroundingTileFlags::LEFT)) {
		return AutoTilePatternFlag::OUTER_MID_RIGHT;
	}
	//LEFT_CENTER
	if (hasFlag(mask, SurroundingTileFlags::RIGHT) && hasFlag(mask, SurroundingTileFlags::DOWN)
		&& hasFlag(mask, SurroundingTileFlags::UP) && !hasFlag(mask, SurroundingTileFlags::LEFT)) {
		return AutoTilePatternFlag::OUTER_MID_LEFT;
	}
	//TOP_CENTER
	if (hasFlag(mask, SurroundingTileFlags::RIGHT) && hasFlag(mask, SurroundingTileFlags::DOWN)
		&& !hasFlag(mask, SurroundingTileFlags::UP) && hasFlag(mask, SurroundingTileFlags::LEFT)) {
		return AutoTilePatternFlag::OUTER_TOP_MID;
	}
	//BOTTOM_LEFT
	if (hasFlag(mask, SurroundingTileFlags::RIGHT) && !hasFlag(mask, SurroundingTileFlags::DOWN)
		&& hasFlag(mask, SurroundingTileFlags::UP) && !hasFlag(mask, SurroundingTileFlags::LEFT)) {
		return AutoTilePatternFlag::OUTER_BOT_LEFT;
	}
	//BOTTOM_RIGHT
	if (!hasFlag(mask, SurroundingTileFlags::RIGHT) && !hasFlag(mask, SurroundingTileFlags::DOWN)
		&& hasFlag(mask, SurroundingTileFlags::UP) && hasFlag(mask, SurroundingTileFlags::LEFT)) {
		return AutoTilePatternFlag::OUTER_BOT_RIGHT;
	}
	//BOTTOM_CENTER
	if (hasFlag(mask, SurroundingTileFlags::RIGHT) && !hasFlag(mask, SurroundingTileFlags::DOWN)
		&& hasFlag(mask, SurroundingTileFlags::UP) && hasFlag(mask, SurroundingTileFlags::LEFT)) {
		return AutoTilePatternFlag::OUTER_BOT_MID;
	}
	//BOTTOM_LEFT_INVERSE
	if (hasFlag(mask, SurroundingTileFlags::RIGHT) && hasFlag(mask, SurroundingTileFlags::UP)
		&& hasFlag(mask, SurroundingTileFlags::LEFT) && !hasFlag(mask, SurroundingTileFlags::UP_RIGHT) && diagonalSum >= 3) {
		return AutoTilePatternFlag::INNER_BOT_LEFT;
	}
	//BOTTOM_RIGHT_INVERSE
	if (hasFlag(mask, SurroundingTileFlags::RIGHT) && hasFlag(mask, SurroundingTileFlags::UP)
		&& hasFlag(mask, SurroundingTileFlags::LEFT) && !hasFlag(mask, SurroundingTileFlags::UP_LEFT) && diagonalSum >= 3) {
		return AutoTilePatternFlag::INNER_BOT_RIGHT;
	}
	//TOP_LEFT_INVERSE
	if (hasFlag(mask, SurroundingTileFlags::RIGHT) && hasFlag(mask, SurroundingTileFlags::UP)
		&& hasFlag(mask, SurroundingTileFlags::LEFT) &&
		!hasFlag(mask, SurroundingTileFlags::DOWN_RIGHT) && diagonalSum >= 3) {
		return AutoTilePatternFlag::INNER_TOP_LEFT;
	}
	//TOP_RIGHT_INVERSE
	if (hasFlag(mask, SurroundingTileFlags::RIGHT) && hasFlag(mask, SurroundingTileFlags::UP)
		&& hasFlag(mask, SurroundingTileFlags::LEFT) &&
		!hasFlag(mask, SurroundingTileFlags::DOWN_LEFT) && diagonalSum >= 3) {
		return AutoTilePatternFlag::INNER_TOP_RIGHT;
	}

	return AutoTilePatternFlag::CENTER;
}
