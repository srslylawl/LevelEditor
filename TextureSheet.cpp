#include "TextureSheet.h"

#include <iostream>

#include "Resources.h"
#include "Serialization.h"
#include "Texture.h"
#include "ImGuiHelper.h"

using namespace Rendering;

bool TextureSheet::CreateNew(const std::filesystem::path& relativePathToImageFile, TextureSheet*& out_TextureSheet, AssetHeader& out_header) {
	Texture* mainTex;
	if (!Texture::CreateNew(relativePathToImageFile, false, true, mainTex, out_header)) {
		return false;
	}
	auto assetPath = relativePathToImageFile;
	out_TextureSheet = new TextureSheet(mainTex, assetPath, AssetId::CreateNewAssetId());
	out_header.aId = out_TextureSheet->AssetId;
	out_header.aType = AssetType::TextureSheet;
	out_header.relativeAssetPath = out_TextureSheet->GetRelativeAssetPath();
	out_TextureSheet->SaveToFile();

	return true;
}

bool TextureSheet::Deserialize(std::istream& iStream, const AssetHeader& header, TextureSheet*& out_textureSheet) {
	using namespace Serialization;
	//embedded here is the metadata of its mainTexture
	AssetHeader textureHeader;
	if (!AssetHeader::Read(iStream, &textureHeader)) {
		std::cout << "Unable to read header of mainTexture asset of textureSheet " << header.relativeAssetPath << std::endl;
		return false;
	}
	Texture* mainTexture;
	if (!Texture::Deserialize(iStream, textureHeader, mainTexture)) {
		std::cout << "Unable to deserialize mainTexture asset - unable to load textureSheet " << header.relativeAssetPath << std::endl;
		return false;
	}
	const auto imgFilePath = mainTexture->GetImageFilePath();
	out_textureSheet = new TextureSheet(mainTexture, imgFilePath, header.aId);
	size_t subTextureCount = 0;
	readFromStream(iStream, subTextureCount);
	if (subTextureCount == 0) {
		return true;
	}
	out_textureSheet->SubTextureData.reserve(subTextureCount);

	for (size_t i = 0; i < subTextureCount; ++i) {
		out_textureSheet->SubTextureData.emplace_back(DeserializeSubTextureData(iStream));
	}

	if (!mainTexture->CreateSubTextures(out_textureSheet->SubTextureData, out_textureSheet->SubTextures)) {
		delete out_textureSheet;
		std::cout << "ERROR: Unable to create texture sheet: failed to create subTextures " << header.relativeAssetPath.string() << std::endl;
		return false;
	}

	return true;
}

void TextureSheet::Serialize(std::ostream& oStream) const {
	using namespace Serialization;
	//Write embedded mainTex metadata first
	AssetHeader::Write(oStream, mainTexture);
	mainTexture->Serialize(oStream);
	writeToStream(oStream, SubTextureData.size());

	for (auto data : SubTextureData) {
		Serialization::Serialize(oStream, data);
	}
}

void TextureSheet::AutoSlice() {
	auto props = mainTexture->GetImageProperties();

	if (props.height % sliceHeight != 0) {
		std::cout << "TextureSheet's HEIGHT is not divisible by sliceHeight, will ignore edges." << std::endl;
	}
	if (props.width % sliceWidth != 0) {
		std::cout << "TextureSheet's WIDTH is not divisible by sliceWidth, will ignore edges." << std::endl;
	}
	const size_t x = props.width / sliceWidth;
	const size_t y = props.height / sliceHeight;
	const size_t newSubTextureCount = x * y;
	const size_t oldSubTextureCount = SubTextureData.size();
	// don't delete all textures as we might be able to re-use some of them when slicing multiple times
	if (oldSubTextureCount > newSubTextureCount) {
		for (auto index = newSubTextureCount - 1; index < oldSubTextureCount; ++index) {
			auto tex = SubTextures[index];
			Resources::ReleaseOwnership(tex, true);
		}
	}
	SubTextures.resize(newSubTextureCount);
	SubTextures.resize(newSubTextureCount);

	int i = 0;
	for (int row = 0; row < y; ++row) {
		for (int column = 0; column < x; ++column) {
			const int xOffset = column * sliceWidth;
			const int yOffset = row * sliceHeight;
			SubTextureData[i].xOffset = xOffset;
			SubTextureData[i].yOffset = yOffset;
			SubTextureData[i].width = sliceWidth;
			SubTextureData[i].width = sliceHeight;
			if (i >= oldSubTextureCount) {
				SubTextureData[i].assetId = AssetId::CreateNewAssetId();
			}
			//SubTextureData.emplace_back(xOffset, yOffset, sliceWidth, sliceHeight);
			++i;
		}
	}

	mainTexture->CreateSubTextures(SubTextureData, SubTextures);
	SaveToFile();
}

bool DrawSubSpriteButton(Texture*& texture, int buttonSize, bool shouldHighlight = false) {
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

bool TextureSheet::RenderEditWindow(FileEditWindow* editWindow, bool isNewFile) {
	using namespace ImGui;
	// Assuming this is inside some window
	auto props = mainTexture->GetImageProperties();
	std::string text(mainTexture->Name + " " + std::to_string(props.width) + " x " + std::to_string(props.height));
	TextUnformatted(text.c_str());
	SliderInt("Zoom", &zoomLevel, 1, 10);
	SameLine();
	if (Button("Save")) {
		SaveToFile();
		return true;
	}
	SameLine();
	if (Button("Cancel")) {
		if (isNewFile) return true;
		TextureSheet* t;
		if (!LoadFromFile(GetRelativeAssetPath().string().c_str(), t)) {
			std::cerr << "Unable to load textureSheet when cancelling editing " << GetRelativeAssetPath() << std::endl;
			return true;
		}

		*this = std::move(*t);
		return true;
	}
	if (TreeNode("AutoSlice")) {
		InputInt("Sprite Width", &sliceWidth);
		InputInt("Sprite Height", &sliceHeight);
		if (BeginPopup("AutoSlice Confirmation")) {
			TextUnformatted("Are you sure? - This will overwrite any manual changes.");
			if (Button("Don't overwrite my changes!")) {
				CloseCurrentPopup();
			}
			SameLine();
			if (Button("Yes, AutoSlice it!")) {
				AutoSlice();
			}
			EndPopup();
		}


		if (Button("AutoSlice")) {
			OpenPopup("AutoSlice Confirmation");
		}
		TreePop();
	}

	const auto startPos = GetCursorScreenPos();
	ImGuiHelper::Image(mainTexture->GetTextureID(), ImVec2(mainTexture->GetImageProperties().width * zoomLevel, mainTexture->GetImageProperties().height * zoomLevel));
	int removeAtPos = -1;
	for (size_t i = 0; i < SubTextureData.size(); ++i) {
		auto& data = SubTextureData[i];
		auto pos = ImVec2(startPos.x + data.xOffset * zoomLevel, startPos.y + data.yOffset * zoomLevel);
		bool isHovered = false;
		bool isHeld = false;
		ImGuiHelper::RectButton(pos, ImVec2(data.width * zoomLevel, data.height * zoomLevel), std::to_string(i).c_str(), &isHovered, &isHeld);
		ImGuiHelper::DragSourceTexture(SubTextures[i]);

		if (isHovered) {
			BeginTooltip();
			TextUnformatted(SubTextures[i]->Name.c_str());
			Separator();
			BeginDisabled(true);
			TextUnformatted("Click and drag to assign texture elsewhere.");
			TextUnformatted("Right-Click to edit");
			TextUnformatted("Press 'X' to delete.");
			EndDisabled();
			EndTooltip();

			if (IsKeyPressed(ImGuiKey_X, false)) {
				removeAtPos = i;
			}

		}
		if (BeginPopupContextItem()) {
			//Edit this sprite
			BeginDisabled();
			TextUnformatted("Hit 'Modify' to apply changes and generate texture.");
			EndDisabled();
			TextUnformatted("Offset:");
			InputInt("x", &data.xOffset);
			InputInt("y", &data.yOffset);
			TextUnformatted("Size:");
			InputInt("width", &data.width);
			InputInt("height", &data.height);
			if (Button("Modify")) {
				//Ugly
				std::vector<Rendering::SubTextureData> stData{ SubTextureData[i] };
				std::vector<Rendering::Texture*> stTex{ SubTextures[i] };
				mainTexture->CreateSubTextures(stData, stTex);
				SubTextureData[i] = stData[0];
			}
			EndPopup();
		}
	}

	if (removeAtPos > -1) {
		auto texIt = SubTextures.begin() + removeAtPos;
		Resources::ReleaseOwnership(*texIt, true);
		SubTextures.erase(texIt);
		SubTextureData.erase(SubTextureData.begin() + removeAtPos);
	}

	return false;

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

