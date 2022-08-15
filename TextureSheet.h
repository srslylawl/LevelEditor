#pragma once
#include "Assets.h"
#include "Texture.h"

namespace Rendering {
	struct SubTextureData;

	//Represents a Texture that holds multiple individual sprites
	class TextureSheet : public PersistentAsset<TextureSheet> {
	public:
		TextureSheet(const TextureSheet& other) = delete;
		TextureSheet(TextureSheet&& other) noexcept = delete;
		TextureSheet& operator=(const TextureSheet& other) = delete;
		TextureSheet& operator=(TextureSheet&& other) noexcept = delete;
	private:
		const int spriteSize = 16; // in Pixels
		Texture* mainTexture;
		TextureSheet(Texture* mainTexture, const std::filesystem::path& relativePathToAsset, const ::AssetId& assetId) :
			PersistentAsset<TextureSheet>(assetId, AssetType::TextureSheet, relativePathToAsset), mainTexture(mainTexture) {}

	public:
		std::vector<Texture*> SubTextures;
		std::vector<SubTextureData> SubTextureData;

		static bool CreateNew(const std::filesystem::path& relativePathToImageFile, TextureSheet*& out_TextureSheet, AssetHeader& out_header);
		static bool Deserialize(std::istream& iStream, const AssetHeader& header, TextureSheet*& out_textureSheet);
		void Serialize(std::ostream& oStream) const override;

		Texture* GetMainTexture() const { return mainTexture; }
		void AutoSlice();
		void RenderImGuiWindow();

		~TextureSheet() override;
	};
}
