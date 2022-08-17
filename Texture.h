#pragma once

#include <string>
#include <filesystem>

#include "Assets.h"
#include "SubTextureData.h"

namespace Rendering {
	struct ImageProperties {
		int width;
		int height;
		int channelCount;
		int colorProfile;

		bool SetColorProfile();

		ImageProperties() = default;
	};


	class Texture : public PersistentAsset<Texture> {
	public:
		Texture(const Texture& other) = delete;
		Texture(Texture&& other) noexcept = delete;
		Texture& operator=(const Texture& other) = delete;
		Texture& operator=(Texture&& other) noexcept = delete;
	private:
		Texture(unsigned int id, const std::filesystem::path& relativePathToImageFile, ImageProperties imageProperties, ::AssetId assetId, bool isInternal = false, std::string nameSuffix = "");
		//Constructor for Empty Texture
		Texture() : PersistentAsset(AssetId::CreateNewAssetId(), AssetType::TextureInternal, "Internal\\Empty_Default"), imageProperties(ImageProperties()) {
		}
		bool isInternalTexture = false;
		unsigned int textureId = 0;
		ImageProperties imageProperties;
		std::string pathToImageFile;

		const char* subTextureSuffix = "_subTexture_";


		static bool LoadImageData(const std::string& relative_path, ImageProperties& out_imageProperties, unsigned char*& out_rawData, bool flipVertically = true);
		static void BindToGPUAndFreeData(const unsigned int& texture_id, const ImageProperties& imageProperties, unsigned char*& imageData);
		void SliceSubTextureFromData(unsigned char* rawImageData, const ImageProperties& imProps, const SubTextureData& subTextureData, const int
		                             subTextureCount, Texture*& out_TexturePtr) const;

		inline static Texture* empty = nullptr;
		static bool Create(const std::filesystem::path& relativePathToImageFile, Texture*& out_texture, bool isInternal, ::AssetId assetId);
		bool LoadAndBind(const std::string& relativePathToImageFile, ImageProperties& out_imgProps, unsigned out_textureId);
		static Texture* CreateFromData(unsigned char* rawImageData, const ImageProperties& imgProps, const std::filesystem::path& relativePathToImageFile,
									   const ::AssetId& assetId, bool isInternal, std::string nameSuffix = "");
		void RefreshFromDataAndFree(unsigned char* rawImageData, const ImageProperties& imgProps);

	public:
		ImageProperties GetImageProperties() const;
		unsigned int GetTextureID() const;

		static bool CreateNew(const std::filesystem::path& relativePathToImageFile, bool isInternal, bool isPartOfTextureSheet, Texture*& out_texture, AssetHeader&
							  out_assetHeader);
		bool CreateSubTextures(const std::vector<SubTextureData>& subTextureData, std::vector<Texture*>& out_textures) const;
		static bool CanCreateFromPath(const char* path);
		bool IsInternal() const {
			return isInternalTexture;
		}


		static Texture* Empty() {
			if (empty == nullptr) empty = new Texture();
			return empty;
		}

		void Serialize(std::ostream& oStream) const override;
		static bool Deserialize(std::istream& iStream, const AssetHeader& header, Texture*& out_texture);

		bool Refresh();

		~Texture() override;

		std::string GetImageFilePath() const {
			return pathToImageFile;
		}
	};
}
