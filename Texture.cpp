#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION //required by stb lib to be in a .cpp file -- DONT REMOVE THIS
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>
#include "glad.h"
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

bool Texture::Load(const std::string& relative_path, ImageProperties& out_imageProperties, unsigned char*& out_rawData, bool flipVertically) {
	stbi_set_flip_vertically_on_load(flipVertically);
	const std::filesystem::path absolutePath = Files::GetAbsolutePath(relative_path);
	//Force 4 color channels so ImGui doesn't die
	out_rawData = stbi_load(absolutePath.string().c_str(), &out_imageProperties.width, &out_imageProperties.height, &out_imageProperties.channelCount, 4);
	out_imageProperties.channelCount = 4;
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
}

ImageProperties Texture::GetImageProperties() const { return imageProperties; }
unsigned Texture::GetTextureID() const { return textureId; }
std::string Texture::GetFilePath() const { return path; }
std::string Texture::GetFileName() const { return name; }

bool Texture::Create(const std::string& relativePath, Texture*& out_texture) {
	ImageProperties imageProperties{};
	unsigned char* rawImageData = nullptr;

	if (!Load(relativePath, imageProperties, rawImageData)) return false;

	unsigned int textureId;
	glGenTextures(1, &textureId);
	BindToGPU(textureId, imageProperties, *rawImageData);
	free(rawImageData);

	const std::filesystem::path p = relativePath;
	const std::string fileName = p.filename().string();
	std::cout << "Image " << fileName << " relativePath: " << relativePath.c_str() << " bound to textureID: " << textureId << std::endl;
	out_texture = new Texture(textureId, p.filename().string(), relativePath, imageProperties);

	return true;
}

bool Texture::CreateSubTexture(const std::string& relativePath, Texture*& out_texture, int xOffset, int yOffset, int subTextureWidth, int subTextureHeight) {
	unsigned char* rawImageData = nullptr;
	ImageProperties imageProperties{};
	//load without flip so yOffset is from the top instead of the bottom as expected
	if(!Load(relativePath, imageProperties, rawImageData, false)) return false;
	// Verify that the chosen area is valid
	if(imageProperties.width < xOffset + subTextureWidth || imageProperties.height < yOffset + subTextureHeight) {
		std::cout << "ERROR: Can't create Subtexture from " << relativePath << " :" << "Subtexture does not fit within Texture!" << std::endl;
		return false;
	}


	const size_t pixelByteSize = imageProperties.channelCount;
	const size_t subTextureSize = pixelByteSize * subTextureHeight * subTextureWidth;
	auto* subTextureData = new unsigned char[subTextureSize];
	// copies whole rows minus offsets into new SubTextureData
	// since whole rows are copied, destination offset is always row-yOffset * subTextureWidth
	const size_t originRowByteSize = pixelByteSize * imageProperties.width;
	const size_t newRowByteSize = pixelByteSize * subTextureWidth;
	for (size_t row = yOffset; row < yOffset+subTextureHeight; row++) {
		const auto rowOffset = row*originRowByteSize;
		const auto columnOffset = xOffset*pixelByteSize;
		const auto srcOffset = rowOffset + columnOffset;
		const auto dstOffset = (row-yOffset) * newRowByteSize;
		memcpy(subTextureData+dstOffset, rawImageData+srcOffset, newRowByteSize);
	}

	imageProperties.width = subTextureWidth;
	imageProperties.height = subTextureHeight;

	const std::filesystem::path p = relativePath;
	//TODO: temp fix
	const std::filesystem::path parentPath = p.parent_path();
	const std::string fileName = p.stem().string() + "_subtexture" + p.extension().string();
	const std::filesystem::path newPath = parentPath.string() + "\\" +fileName;


	stbi_write_png(newPath.string().c_str(), imageProperties.width, imageProperties.height, imageProperties.channelCount, subTextureData, static_cast<int>(newRowByteSize));

	//flip now before passing to GPU
	stbi__vertical_flip(subTextureData, subTextureWidth, subTextureHeight, static_cast<int>(pixelByteSize));

	unsigned int textureId;
	glGenTextures(1, &textureId);
	BindToGPU(textureId, imageProperties, *subTextureData);
	free(rawImageData);
	delete[] subTextureData;

	std::cout << "SubTexture " << newPath.filename().string() << " relativePath: " << newPath.string().c_str() << " bound to textureID: " << textureId << std::endl;
	out_texture = new Texture(textureId, newPath.filename().string(), newPath.string(), imageProperties);

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
