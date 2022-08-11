#pragma once
#include "Asset.h"
#include "Strings.h"
#include "Texture.h"

namespace Rendering {
	struct SubTextureData;
	class Texture;
}

//Represents a Texture that holds multiple individual sprites
class TextureSheet : public StandaloneAsset<TextureSheet> {
public:
	TextureSheet(const TextureSheet& other) = delete;
	TextureSheet(TextureSheet&& other) noexcept = delete;
	TextureSheet& operator=(const TextureSheet& other) = delete;
	TextureSheet& operator=(TextureSheet&& other) noexcept = delete;
private:
	const int spriteSize = 16; // in Pixels
	Rendering::Texture* mainTexture = nullptr;

	explicit TextureSheet(Rendering::Texture* mainTexture) : mainTexture(mainTexture) {
		const std::filesystem::path texFilePath = mainTexture->Name;
		Name = texFilePath.stem().string();
	}
public:
	std::vector<Rendering::Texture*> SubTextures;
	std::vector<Rendering::SubTextureData> SubTextureData;

	static TextureSheet* CreateNew(Rendering::Texture* mainTexture);

	Rendering::Texture* GetMainTexture() const { return mainTexture == nullptr ? Rendering::Texture::Empty() : mainTexture; }

	static bool Deserialize(std::istream& iStream, TextureSheet*& out_textureSheet);
	void Serialize(std::ostream& oStream) const override;

	void AutoSlice();
	void RenderImGuiWindow();

	~TextureSheet() override;
};

template<> inline const std::string StandaloneAsset<TextureSheet>::GetFileEnding() {
	return ".texsheet";
}
template<> inline const std::string StandaloneAsset<TextureSheet>::GetParentDirectory() {
	return Strings::Directory_TextureSheets;
}

