#include "TextureSheet.h"

#include <iostream>

#include "Resources.h"
#include "Serialization.h"
#include "Texture.h"
#include "ImGuiHelper.h"


void TextureSheet::CreateNew(Rendering::Texture* mainTexture, AssetHeader& out_header) {
	const auto* sheet = new TextureSheet(mainTexture, AssetId::CreateNewAssetId());
	out_header.aId = sheet->AssetId;
	out_header.aType = AssetType::TextureSheet;
	out_header.relativeAssetPath = sheet->GetRelativePath();
	sheet->SaveToFile();
}

bool TextureSheet::Deserialize(std::istream& iStream, const AssetHeader& header, TextureSheet*& out_textureSheet) {
	using namespace Serialization;
	std::string name = DeserializeStdString(iStream);
	::AssetId mainTexId;
	if(!TryDeserializeAssetId(iStream, mainTexId)) {
		std::cout << "Unable to find mainTexture asset id: '" << name << "' - unable to load TextureSheet" << std::endl;
		return false;
	}
	Rendering::Texture* mainTexture;
	if (!Resources::TryGetTexture(mainTexId, mainTexture)) {
		std::cout << "Unable to find mainTexture: '" << mainTexId.ToString() << "' - unable to load TextureSheet" << std::endl;
		return false;
	}
	out_textureSheet = new TextureSheet(mainTexture, header.aId);
	out_textureSheet->Name = name;
	size_t subTextureCount = 0;
	readFromStream(iStream, subTextureCount);
	if (subTextureCount == 0) {
		return true;
	}

	out_textureSheet->SubTextureData.reserve(subTextureCount);

	for (size_t i = 0; i < subTextureCount; ++i) {
		out_textureSheet->SubTextureData.emplace_back(DeserializeSubTextureData(iStream));
	}

	mainTexture->CreateSubTextures(out_textureSheet->SubTextureData, out_textureSheet->SubTextures);

	std::string fileEndingCheck = DeserializeStdString(iStream);

	bool valid = fileEndingCheck == GetFileEnding();
	if (!valid) {
		delete out_textureSheet;
		std::cout << "File ending check failed." << std::endl;
	}

	return valid;
}

void TextureSheet::Serialize(std::ostream& oStream) const {
	using namespace Serialization;
	Serialization::Serialize(oStream, Name);
	Serialization::Serialize(oStream, mainTexture->AssetId);
	writeToStream(oStream, SubTextureData.size());

	for (auto data : SubTextureData) {
		Serialization::Serialize(oStream, data);
	}

	Serialization::Serialize(oStream, GetFileEnding());
}

void TextureSheet::AutoSlice() {
	auto props = mainTexture->GetImageProperties();

	if (props.height % spriteSize != 0) {
		std::cout << "TextureSheet's HEIGHT is not divisible by spriteSize, abort." << std::endl;
		return;
	}
	if (props.width % spriteSize != 0) {
		std::cout << "TextureSheet's WIDTH is not divisible by SpriteSize, abort." << std::endl;
		return;
	}
	const size_t x = props.width / spriteSize;
	const size_t y = props.height / spriteSize;

	//TODO: dont delete textures as we might be able to re-use some of them when slicing multiple times
	for (auto& subTexture : SubTextures) {
		delete subTexture;
	}
	SubTextures.clear();
	SubTextureData.clear();
	SubTextures.reserve(x * y);
	SubTextures.reserve(x * y);


	for (int row = 0; row < y; ++row) {
		for (int column = 0; column < x; ++column) {
			int xOffset = column * spriteSize;
			int yOffset = row * spriteSize;
			SubTextureData.emplace_back(xOffset, yOffset, spriteSize, spriteSize);
		}
	}

	mainTexture->CreateSubTextures(SubTextureData, SubTextures);
	SaveToFile();
}

bool DrawSubSpriteButton(Rendering::Texture*& texture, int buttonSize, bool shouldHighlight = false) {
	using namespace ImGui;
	//	ImVec2 startPos = GetCursorStartPos();

	//const int iconSideLength = size;
	//const int totalItemWidth = iconSideLength + 3;
	//startPos.x += (startPos.x + totalItemWidth) * elementCount;

	//ImVec2 IconDrawPos(startPos.x, startPos.y);
	//SetCursorPos(IconDrawPos);
	if (shouldHighlight) PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(255, 255, 255));
	const bool clicked = ImGuiHelper::ImageButton(texture->GetTextureID(), ImVec2(buttonSize, buttonSize), 0);
	if (shouldHighlight) PopStyleColor();

	PopID();

	ImGuiHelper::DragSourceTexture(texture);

	if (IsItemHovered()) {
		BeginTooltip();
		TextUnformatted(texture->Name.c_str());
		EndTooltip();
	}

	return clicked;
}

void TextureSheet::RenderImGuiWindow() {
	using namespace ImGui;
	// Assuming this is inside some window
	auto props = mainTexture->GetImageProperties();
	std::string text(mainTexture->Name + " " + std::to_string(props.width) + " x " + std::to_string(props.height));
	TextUnformatted(text.c_str());
	static int zoomLevel = 1;
	SliderInt("Zoom", &zoomLevel, 1, 10);
	SameLine();
	if (Button("Save")) {
		SaveToFile();
	}

	const auto startPos = GetCursorScreenPos();
	ImGuiHelper::Image(mainTexture->GetTextureID(), ImVec2(mainTexture->GetImageProperties().width * zoomLevel, mainTexture->GetImageProperties().height * zoomLevel));
	int removeAtPos = -1;
	for (size_t i = 0; i < SubTextureData.size(); ++i) {
		auto data = SubTextureData[i];
		auto pos = ImVec2(startPos.x + data.xOffset * zoomLevel, startPos.y + data.yOffset * zoomLevel);
		bool isHovered = false;
		bool isHeld = false;
		ImGuiHelper::RectButton(pos, ImVec2(data.width * zoomLevel, data.height * zoomLevel), std::to_string(i).c_str(), &isHovered, &isHeld);
		if (isHovered) {
			BeginTooltip();
			TextUnformatted("Press 'X' to delete.");
			EndTooltip();

			if (IsKeyPressed(ImGuiKey_X, false)) {
				removeAtPos = i;
			}
		}
	}

	if (removeAtPos > -1) {
		SubTextureData.erase(SubTextureData.begin() + removeAtPos);
		SubTextures.erase(SubTextures.begin() + removeAtPos);
	}


	//size_t total = SubTextures.size();
	//size_t rows = total * spriteSize / mainTexture->GetImageProperties().height;
	//int buttonSize = 32;
	//for (size_t i = 0; i < SubTextures.size(); ++i) {
	//	if (i % rows != 0) {
	//		SameLine();
	//	}
	//	PushID(i);
	//	DrawSubSpriteButton(SubTextures[i], buttonSize);
	//}
}

TextureSheet::~TextureSheet() = default;
