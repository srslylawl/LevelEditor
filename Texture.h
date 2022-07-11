#pragma once

#include <string>
#include <filesystem>

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


	class Texture {
	public:
		Texture(const Texture& other) = delete;
		Texture(Texture&& other) noexcept = delete;
		Texture& operator=(const Texture& other) = delete;
		Texture& operator=(Texture&& other) noexcept = delete;

	private:
		Texture(unsigned id, std::string name, std::string path, ImageProperties imageProperties);

		unsigned int textureId = 0;
		ImageProperties imageProperties;
		std::string relativeFilePath;
		std::string fileName;

		const char* subTextureSuffix = "_subTexture_";

		static bool Load(const std::string& relative_path, ImageProperties& out_imageProperties, unsigned char*& out_rawData, bool flipVertically = true);

		static void BindToGPU(const unsigned int& texture_id, const ImageProperties& imageProperties, unsigned char*& imageData);
		void SliceSubTextureFromData(unsigned char* rawImageData, ImageProperties imProps, SubTextureData subTextureData, std::filesystem
		                             ::path newTexturePath, Texture*& out_TexturePtr);

		inline static Texture* empty = nullptr;
	public:

		ImageProperties GetImageProperties() const;
		unsigned int GetTextureID() const;
		std::string GetRelativeFilePath() const;
		std::string GetFileName() const;

		static bool Create(const std::string& relativePath, Texture*& out_texture);

		bool CreateSubTextures(std::vector<SubTextureData>& subTextureData, std::vector<Texture*>& out_textures);

		static bool CanCreateFromPath(const char* path);

		static Texture* Empty() {
			if(empty == nullptr) {
				empty = new Texture(0, "Empty_Default", "", ImageProperties());
			}
			return empty;
		}

		bool Refresh();

		~Texture();
	};
}


