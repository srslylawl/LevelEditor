#pragma once
#include "Assets.h"
#include "Strings.h"
#include "Texture.h"

namespace Rendering {
	struct SubTextureData;
	class Texture;
}

//Represents a Texture that holds multiple individual sprites
class TextureSheet : public PersistentAsset<TextureSheet> {
public:
	TextureSheet(const TextureSheet& other) = delete;
	TextureSheet(TextureSheet&& other) noexcept = delete;
	TextureSheet& operator=(const TextureSheet& other) = delete;
	TextureSheet& operator=(TextureSheet&& other) noexcept = delete;
private:
	const int spriteSize = 16; // in Pixels
	Rendering::Texture* mainTexture = nullptr;

	TextureSheet(Rendering::Texture* mainTexture, ::AssetId id) :
		PersistentAsset(id, AssetType::TextureSheet, mainTexture->Name),
		mainTexture(mainTexture) { }
public:
	std::vector<Rendering::Texture*> SubTextures;
	std::vector<Rendering::SubTextureData> SubTextureData;

	static void CreateNew(Rendering::Texture* mainTexture, AssetHeader& out_header);

	Rendering::Texture* GetMainTexture() const {
		return mainTexture == nullptr ? Rendering::Texture::Empty() : mainTexture;
	}

	static bool Deserialize(std::istream& iStream, const AssetHeader& header, TextureSheet*& out_textureSheet);
	void Serialize(std::ostream& oStream) const override;

	void AutoSlice();
	void RenderImGuiWindow();

	~TextureSheet() override;
};

template<> inline const std::string PersistentAsset<TextureSheet>::GetFileEnding() {
	return ".texsheet";
}
template<> inline const std::string PersistentAsset<TextureSheet>::GetParentDirectory() {
	return Strings::Directory_TextureSheets;
}

