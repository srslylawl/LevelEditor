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

		static bool Load(const std::string& path, ImageProperties& out_imageProperties, unsigned char*& out_rawData);

		static void BindToGPU(const unsigned int& texture_id, const ImageProperties& imageProperties, unsigned char& imageData);
	public:

		ImageProperties GetImageProperties() const;
		unsigned int GetTextureID() const;
		std::string GetFilePath() const;
		std::string GetFileName() const;

		static bool Create(const std::string& path, Texture*& out_texture);

		static bool CanCreateFromPath(const char* path);

		bool Refresh();

		~Texture();
	};
}


