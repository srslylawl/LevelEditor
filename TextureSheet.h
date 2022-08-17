#pragma once
#include "Assets.h"
#include "FileEditWindow.h"
#include "Texture.h"

namespace Rendering {
	struct SubTextureData;

	//Represents a Texture that holds multiple individual sprites
	class TextureSheet : public PersistentAsset<TextureSheet>, public IEditable {
	public:
		TextureSheet(const TextureSheet& other) = delete;
		TextureSheet& operator=(const TextureSheet& other) = delete;

		TextureSheet(TextureSheet&& other) noexcept = default;
		TextureSheet& operator=(TextureSheet&& other) noexcept {
			if(this == &other) return *this;
			mainTexture = other.mainTexture;
			sliceHeight = other.sliceHeight;
			sliceWidth = other.sliceWidth;
			SubTextures = std::move(other.SubTextures);
			SubTextureData = std::move(other.SubTextureData);

			return *this;
		}
	private:
		int sliceWidth = 16; // in Pixels
		int sliceHeight = 16; // in Pixels
		int zoomLevel = 2;
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

		~TextureSheet() override;
		bool RenderEditWindow(FileEditWindow* editWindow, bool isNewFile) override;
		std::filesystem::path IEditableGetAssetPath() override { return GetRelativeAssetPath(); }
	};
}
