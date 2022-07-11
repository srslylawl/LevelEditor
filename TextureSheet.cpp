#include "TextureSheet.h"

#include <iostream>

#include "Resources.h"
#include "Serialization.h"
#include "Texture.h"


bool TextureSheet::Deserialize(std::istream& iStream, TextureSheet*& out_textureSheet) {
	std::string mainTextureStr = Serialization::DeserializeStdString(iStream);
	Rendering::Texture* mainTexture;
	if (!Resources::TryGetTexture(mainTextureStr.c_str(), mainTexture)) {
		std::cout << "Unable to find mainTexture: '" << mainTexture << "' - unable to load TextureSheet" << std::endl;
		return false;
	}
	out_textureSheet = new TextureSheet(mainTexture);
	size_t subTextureCount = 0;
	Serialization::readFromStream(iStream, subTextureCount);
	if (subTextureCount == 0) {
		return true;
	}

	out_textureSheet->SubTextureData.reserve(subTextureCount);

	for (size_t i = 0; i < subTextureCount; ++i) {
		out_textureSheet->SubTextureData.emplace_back(Serialization::DeserializeSubTextureData(iStream));
	}

	mainTexture->CreateSubTextures(out_textureSheet->SubTextureData, out_textureSheet->SubTextures);

	std::string fileEndingCheck = Serialization::DeserializeStdString(iStream);

	bool valid = fileEndingCheck == FileEnding;
	if (!valid) {
		delete out_textureSheet;
		std::cout << "File ending check failed." << std::endl;
	}

	return valid;
}

void TextureSheet::Serialize(std::ostream& oStream) const {
	Serialization::writeToStream(oStream, mainTexture->GetRelativeFilePath());
	Serialization::writeToStream(oStream, SubTextureData.size());

	for (auto data : SubTextureData) {
		Serialization::Serialize(oStream, data);
	}

	Serialization::Serialize(oStream, FileEnding);
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
	const size_t x = props.height / spriteSize;
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
}

TextureSheet::~TextureSheet() {
	for (auto& subTexture : SubTextures) {
		delete subTexture;
	}
}
