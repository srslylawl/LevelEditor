#pragma once

#include <string>
#include <filesystem>

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
		std::string path;
		std::string name;

		static bool Load(const std::string& relative_path, ImageProperties& out_imageProperties, unsigned char*& out_rawData, bool flipVertically = true);

		static void BindToGPU(const unsigned int& texture_id, const ImageProperties& imageProperties, unsigned char& imageData);
		inline static Texture* empty = nullptr;
	public:

		ImageProperties GetImageProperties() const;
		unsigned int GetTextureID() const;
		std::string GetFilePath() const;
		std::string GetFileName() const;

		static bool Create(const std::string& relativePath, Texture*& out_texture);
		static bool CreateSubTexture(const std::string& relativePath, Texture*& out_texture, int xOffset, int yOffset, int subTextureWidth, int subTextureHeight);

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


