#pragma once
#include "Strings.h"
#include "Texture.h"

namespace Rendering {
	struct SubTextureData;
	class Texture;
}

//Represents a Texture that holds multiple individual sprites
class TextureSheet {
public:
	TextureSheet(const TextureSheet& other) = delete;
	TextureSheet(TextureSheet&& other) noexcept = delete;
	TextureSheet& operator=(const TextureSheet& other) = delete;
	TextureSheet& operator=(TextureSheet&& other) noexcept = delete;
private:
	const int spriteSize = 16; // in Pixels
	Rendering::Texture* mainTexture;

	std::string Name;
	inline static const std::string ParentDirectory = Strings::Directory_TextureSheets;
public:
	inline static const std::string FileEnding = ".texsheet";

	std::vector<Rendering::Texture*> SubTextures;
	std::vector<Rendering::SubTextureData> SubTextureData;

	explicit TextureSheet(Rendering::Texture* mainTexture) : mainTexture(mainTexture) {
		Name = mainTexture->GetFileName();
	}

	static bool Deserialize(std::istream& iStream, TextureSheet*& out_textureSheet);
	void Serialize(std::ostream& oStream) const;

	void AutoSlice();

	~TextureSheet();
};

