#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION //required by stb lib to be in a .cpp file -- DONT REMOVE THIS

#include <iostream>
#include "glad.h"
#include "stb_image.h"
#include "Files.h"

using namespace Rendering;

inline bool ImageProperties::SetColorProfile() {
	switch (channelCount) {
	case 3:
		colorProfile = GL_RGB;
		break;
	case 4:
		colorProfile = GL_RGBA;
		break;
	default:
		std::cout << "Color Profile " << channelCount << " not recognized" << std::endl;
		return false;
	}
	return true;
}

Texture::Texture(const unsigned id, std::string name, std::string path,
	const ImageProperties imageProperties) : textureId(id),
	imageProperties(imageProperties),
	path(std::move(path)),
	name(std::move(name)) {
	std::cout << "Created Texture " << this->name << ": Path: " << this->path << std::endl;
}

bool Texture::Load(const std::string& relative_path, ImageProperties& out_imageProperties, unsigned char*& out_rawData) {
	stbi_set_flip_vertically_on_load(true);
	const std::filesystem::path absolutePath = Files::GetAbsolutePath(relative_path);
	out_rawData = stbi_load(absolutePath.string().c_str(), &out_imageProperties.width, &out_imageProperties.height, &out_imageProperties.channelCount, 0);
	const bool success = out_rawData != nullptr;
	if (!success) {
		std::cout << "Unable to load image: " << relative_path << " : " << stbi_failure_reason() << std::endl;
		return false;
	}
	if (!out_imageProperties.SetColorProfile()) return false;
	return true;
}

void Texture::BindToGPU(const unsigned int& texture_id, const ImageProperties& imageProperties, unsigned char& imageData) {
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, imageProperties.colorProfile, imageProperties.width, imageProperties.height, 0, imageProperties.colorProfile, GL_UNSIGNED_BYTE, &imageData);
	glGenerateMipmap(GL_TEXTURE_2D);

	free(&imageData);
}

ImageProperties Texture::GetImageProperties() const { return imageProperties; }
unsigned Texture::GetTextureID() const { return textureId; }
std::string Texture::GetFilePath() const { return path; }
std::string Texture::GetFileName() const { return name; }

bool Texture::Create(const std::string& path, Texture*& out_texture) {
	ImageProperties imageProperties{};
	unsigned char* rawImageData = nullptr;

	if (!Load(path, imageProperties, rawImageData)) return false;

	unsigned int textureId;
	glGenTextures(1, &textureId);
	BindToGPU(textureId, imageProperties, *rawImageData);

	const std::filesystem::path p = path;
	const std::string fileName = p.filename().string();
	std::cout << "Image " << fileName << " path: " << path.c_str() << " bound to textureID: " << textureId << std::endl;
	out_texture = new Texture(textureId, p.filename().string(), path, imageProperties);

	return true;
}
bool Texture::CanCreateFromPath(const char* path) {
	int _;
	const int ok = stbi_info(path, &_, &_, &_);
	return ok;
}
bool Texture::Refresh() {
	unsigned char* rawImageData = nullptr;
	if (!Load(path, imageProperties, rawImageData)) return false;
	BindToGPU(textureId, imageProperties, *rawImageData);

	std::cout << "Image " << name << " refreshed to textureID: " << textureId << std::endl;

	return true;
}
Texture::~Texture() {
	glDeleteTextures(1, &textureId);
	std::cout << "Image " << name << " deleted." << std::endl;
}
