#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION //required by stb lib to be in a .cpp file -- DONT REMOVE THIS
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>
#include "glad.h"
#include "Files.h"
#include "Resources.h"
#include "Serialization.h"

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

Texture::Texture(const unsigned id, std::string name, std::string path, const ImageProperties imageProperties, bool isInternal) :
	isInternalTexture(isInternal),
	textureId(id),
	imageProperties(imageProperties),
	relativeFilePath(std::move(path)) {
	Name = std::move(name);
	std::cout << "Created Texture " << this->Name << ": Path: " << this->relativeFilePath << std::endl;
	Resources::AddTexture(this, isInternal);
}

bool Texture::LoadImageData(const std::string& relative_path, ImageProperties& out_imageProperties, unsigned char*& out_rawData, bool flipVertically) {
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

void Texture::BindToGPU(const unsigned int& texture_id, const ImageProperties& imageProperties, unsigned char*& imageData) {
	//TODO: allow customization of parameters
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, imageProperties.colorProfile, imageProperties.width, imageProperties.height, 0, imageProperties.colorProfile, GL_UNSIGNED_BYTE, imageData);
	glGenerateMipmap(GL_TEXTURE_2D);
}

ImageProperties Texture::GetImageProperties() const {
	return imageProperties;
}
unsigned Texture::GetTextureID() const {
	return textureId;
}

bool Texture::CreateNew(const std::string& relativePath, Texture*& out_texture, bool isInternal) {
	ImageProperties imageProperties{};
	unsigned char* rawImageData = nullptr;
	if (!LoadImageData(relativePath, imageProperties, rawImageData)) return false;

	unsigned int textureId;
	glGenTextures(1, &textureId);
	BindToGPU(textureId, imageProperties, rawImageData);
	free(rawImageData);

	const std::filesystem::path p = relativePath;
	const std::string fileName = p.filename().string();
	std::cout << "Image " << relativePath.c_str() << " bound to textureID: " << textureId << std::endl;
	out_texture = new Texture(textureId, p.filename().string(), relativePath, imageProperties, isInternal);

	return true;
}

void Texture::SliceSubTextureFromData(unsigned char* rawImageData, const ImageProperties imProps, const SubTextureData& subTextureData,
									  const std::filesystem::path& newTexturePath, Texture*& out_TexturePtr) const {
	const size_t pixelByteSize = imProps.channelCount;
	const size_t originRowByteSize = pixelByteSize * imProps.width;

	const size_t subTextureSize = pixelByteSize * subTextureData.height * subTextureData.width;
	auto* rawSubTextureData = new unsigned char[subTextureSize];
	// copies whole rows minus offsets into new SubTextureData
	// since whole rows are copied, destination offset is always row-yOffset * subTextureWidth
	const size_t newRowByteSize = pixelByteSize * subTextureData.width;

	for (size_t row = subTextureData.yOffset; row < subTextureData.yOffset + subTextureData.height; row++) {
		const auto rowOffset = row * originRowByteSize;
		const auto columnOffset = subTextureData.xOffset * pixelByteSize;
		const auto srcOffset = rowOffset + columnOffset;
		const auto dstOffset = (row - subTextureData.yOffset) * newRowByteSize;
		memcpy(rawSubTextureData + dstOffset, rawImageData + srcOffset, newRowByteSize);
	}

	ImageProperties subTexImgProps{};
	subTexImgProps.width = subTextureData.width;
	subTexImgProps.height = subTextureData.height;
	subTexImgProps.channelCount = imageProperties.channelCount;
	subTexImgProps.colorProfile = imageProperties.colorProfile;

	//flip now before passing to GPU
	stbi__vertical_flip(rawSubTextureData, subTextureData.width, subTextureData.height, static_cast<int>(pixelByteSize));

	unsigned int textureId;
	glGenTextures(1, &textureId);
	BindToGPU(textureId, subTexImgProps, rawSubTextureData);
	delete[] rawSubTextureData;
	//TODO: check if texture already exists for that path, and if yes, refresh instead
	out_TexturePtr = new Texture(textureId, newTexturePath.filename().string(), newTexturePath.string(), imageProperties);
}

bool Texture::CreateSubTextures(std::vector<SubTextureData>& subTextureData, std::vector<Texture*>& out_textures) {
	unsigned char* rawImageData = nullptr;
	ImageProperties imageProperties{};
	//load without flip so yOffset is from the top instead of the bottom as expected
	if (!LoadImageData(GetRelativePath(), imageProperties, rawImageData, false)) return false;

	for (const auto& sData : subTextureData) {
		if (imageProperties.width < sData.xOffset + sData.width || imageProperties.height < sData.yOffset + sData.height) {
			std::cout << "ERROR: Can't create Subtexture from " << GetRelativePath() << " :" << "Subtexture does not fit within Texture!" << std::endl;
			return false;
		}
	}

	const std::filesystem::path p = GetRelativePath();
	const std::filesystem::path parentPath = p.parent_path();

	int subTextureCount = 0;
	for (const auto& sData : subTextureData) {
		Texture* tPtr = nullptr;
		const std::string fileName = p.stem().string() + subTextureSuffix + std::to_string(subTextureCount++) + p.extension().string();
		const std::filesystem::path newPath = parentPath.string() + "\\" + fileName;
		SliceSubTextureFromData(rawImageData, imageProperties, sData, newPath, tPtr);
		out_textures.push_back(tPtr);
		//std::cout << "SubTextureData " << newPath.string().c_str() << " bound to textureID: " << tPtr->GetTextureID() << std::endl;
	}

	//stbi_write_png(newPath.string().c_str(), imageProperties.width, imageProperties.height, imageProperties.channelCount, subTextureData, static_cast<int>(newRowByteSize));

	free(rawImageData);
	return true;
}
bool Texture::CanCreateFromPath(const char* path) {
	int _;
	const int ok = stbi_info(path, &_, &_, &_);
	return ok;
}

void Texture::Serialize(std::ostream& oStream) const {
	AssetHeader::Write(oStream, isInternalTexture ? AssetType::TextureInternal : AssetType::Texture, AssetId);
	Serialization::Serialize(oStream, GetRelativePath());
	Serialization::writeToStream(oStream, isInternalTexture);
}

bool Texture::Deserialize(std::istream& iStream, Texture*& out_texture) {
	AssetHeader header;
	if(!AssetHeader::Deserialize(iStream, &header)) return false;
	std::string relFilePath = Serialization::DeserializeStdString(iStream);
	bool isInternal = false; Serialization::readFromStream(iStream, isInternal);
	return CreateNew(relFilePath, out_texture, isInternal);
}

bool Texture::Refresh() {
	unsigned char* rawImageData = nullptr;
	if (!LoadImageData(relativeFilePath, imageProperties, rawImageData)) return false;
	BindToGPU(textureId, imageProperties, rawImageData);
	delete[] rawImageData;

	std::cout << "Image " << Name << " refreshed to textureID: " << textureId << std::endl;

	return true;
}
Texture::~Texture() {
	glDeleteTextures(1, &textureId);
	std::cout << "Image " << Name << " deleted." << std::endl;
}
