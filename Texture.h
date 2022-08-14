#pragma once

#include <string>
#include <filesystem>

#include "Assets.h"
#include "Strings.h"
#include "SubTextureData.h"

namespace Rendering {
	class Texture;
}

template<> inline const std::string PersistentAsset<Rendering::Texture>::GetFileEnding() {
	return ".tex";
}
template<> inline const std::string PersistentAsset<Rendering::Texture>::GetParentDirectory() {
	return Strings::Directory_Sprites;
}

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
		Texture(unsigned int id, const std::string& name, std::string path, ImageProperties imageProperties, ::AssetId assetId, bool isInternal = false);

		bool isInternalTexture = false;
		unsigned int textureId = 0;
		ImageProperties imageProperties;
		std::string relativeFilePath;

		const char* subTextureSuffix = "_subTexture_";

		static bool LoadImageData(const std::string& relative_path, ImageProperties& out_imageProperties, unsigned char*& out_rawData, bool flipVertically = true);

		static void BindToGPUAndFreeData(const unsigned int& texture_id, const ImageProperties& imageProperties, unsigned char*& imageData);
		void SliceSubTextureFromData(unsigned char* rawImageData, const ImageProperties& imProps, const SubTextureData& subTextureData, Texture*& out_TexturePtr) const;

		inline static Texture* empty = nullptr;
		static bool Create(const std::string& relativePath, Texture*& out_texture, bool isInternal, ::AssetId assetId);
		static Texture* CreateFromData(unsigned char* rawImageData, const ImageProperties& imgProps, const std::string& relativePath,
		                               const ::AssetId& assetId, bool isInternal);
		void RefreshFromDataAndFree(unsigned char* rawImageData, const ImageProperties& imgProps);

	public:
		ImageProperties GetImageProperties() const;
		unsigned int GetTextureID() const;

		static bool CreateNew(const std::string& relativePath, bool isInternal, bool isPartOfTextureSheet, Texture*& out_texture, AssetHeader&
		                      out_assetHeader);
		static bool Load(const std::string& relativePath, Texture*& out_texture, ::AssetId assetId, bool isInternal);
		bool CreateSubTextures(const std::vector<SubTextureData>& subTextureData, std::vector<Texture*>& out_textures) const;
		static bool CanCreateFromPath(const char* path);
		bool IsInternal() const {
			return isInternalTexture;
		}


		static Texture* Empty() {
			if (empty == nullptr) {
				empty = new Texture(0, "Empty_Default", "", ImageProperties(), AssetId::CreateNewAssetId());
			}
			return empty;
		}

		void Serialize(std::ostream& oStream) const override;
		static bool Deserialize(std::istream& iStream, const AssetHeader& header, Texture*& out_texture);

		bool Refresh();

		~Texture() override;


		std::string GetImageFilePath() const { return std::string(isInternalTexture ? Strings::Directory_Resources_Icons : Strings::Directory_Sprites) + "\\" + Name; }
		std::string GetRelativePath() const override {
			if (!isInternalTexture) return PersistentAsset<Texture>::GetRelativePath();
			return std::string(Strings::Directory_Resources_Icons) + "\\" + Name + AssetHeader::FileExtension;
		}

		std::string GetRelativePath(const std::string& nameOverride) const override {
			if (!isInternalTexture) return PersistentAsset<Texture>::GetRelativePath();
			return std::string(Strings::Directory_Resources_Icons) + "\\" + nameOverride + AssetHeader::FileExtension;
		}
	};
}



