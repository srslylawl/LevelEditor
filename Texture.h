#pragma once

#include <string>
#include <filesystem>

#include "Asset.h"
#include "Strings.h"
#include "SubTextureData.h"

namespace Rendering {
	class Texture;
}

template<> inline const std::string StandaloneAsset<Rendering::Texture>::GetFileEnding() {
	return ".tex";
}
template<> inline const std::string StandaloneAsset<Rendering::Texture>::GetParentDirectory() {
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


	class Texture : public StandaloneAsset<Texture> {
	public:
		Texture(const Texture& other) = delete;
		Texture(Texture&& other) noexcept = delete;
		Texture& operator=(const Texture& other) = delete;
		Texture& operator=(Texture&& other) noexcept = delete;
	private:
		Texture(unsigned int id, std::string name, std::string path, ImageProperties imageProperties, bool isInternal = false);

		bool isInternalTexture = false;
		unsigned int textureId = 0;
		ImageProperties imageProperties;
		std::string relativeFilePath;

		const char* subTextureSuffix = "_subTexture_";

		static bool LoadImageData(const std::string& relative_path, ImageProperties& out_imageProperties, unsigned char*& out_rawData, bool flipVertically = true);

		static void BindToGPU(const unsigned int& texture_id, const ImageProperties& imageProperties, unsigned char*& imageData);
		void SliceSubTextureFromData(unsigned char* rawImageData, const ImageProperties imProps, const SubTextureData& subTextureData, const std::filesystem
		                             ::path& newTexturePath, Texture*& out_TexturePtr) const;

		inline static Texture* empty = nullptr;
	public:

		ImageProperties GetImageProperties() const;
		unsigned int GetTextureID() const;

		static bool CreateNew(const std::string& relativePath, Texture*& out_texture, bool isInternal);
		bool CreateSubTextures(std::vector<SubTextureData>& subTextureData, std::vector<Texture*>& out_textures);
		static bool CanCreateFromPath(const char* path);

		static Texture* Empty() {
			if(empty == nullptr) {
				empty = new Texture(0, "Empty_Default", "", ImageProperties());
			}
			return empty;
		}

		void Serialize(std::ostream& oStream) const override;
		static bool Deserialize(std::istream& iStream, Texture*& out_texture);

		bool Refresh();

		~Texture() override;
		std::string GetRelativePath() const override {
			if(!isInternalTexture) return StandaloneAsset<Texture>::GetRelativePath();
			return std::string(Strings::Directory_Icon) + "\\" + Name + GetFileEnding() + AssetHeader::FileExtension;
		}

		std::string GetRelativePath(const std::string& nameOverride) const override {
			if(!isInternalTexture) return StandaloneAsset<Texture>::GetRelativePath();
			return std::string(Strings::Directory_Icon) + "\\" + nameOverride + GetFileEnding() + AssetHeader::FileExtension;
		}
	};
}



