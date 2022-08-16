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

Texture::Texture(unsigned int id, const std::filesystem::path& relativePathToImageFile, ImageProperties imageProperties, ::AssetId assetId, bool isInternal) :
	PersistentAsset(assetId, isInternal ? AssetType::TextureInternal : AssetType::Texture, relativePathToImageFile),
	isInternalTexture(isInternal),
	textureId(id),
	imageProperties(imageProperties),
	pathToImageFile(relativePathToImageFile.string()) {
	std::cout << "Created Texture " << this->Name << ": Path: " << relativePathToImageFile.string() << " assetId: " << AssetId.ToString() << std::endl;
	Resources::AssignOwnership(this);
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

void Texture::BindToGPUAndFreeData(const unsigned int& texture_id, const ImageProperties& imageProperties, unsigned char*& imageData) {
	//TODO: allow customization of parameters
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, imageProperties.colorProfile, imageProperties.width, imageProperties.height, 0, imageProperties.colorProfile, GL_UNSIGNED_BYTE, imageData);
	glGenerateMipmap(GL_TEXTURE_2D);
	free(imageData);
}

ImageProperties Texture::GetImageProperties() const {
	return imageProperties;
}
unsigned Texture::GetTextureID() const {
	return textureId;
}

bool Texture::Create(const std::filesystem::path& relativePathToImageFile, Texture*& out_texture, bool isInternal, ::AssetId assetId) {
	ImageProperties imgProps{};
	unsigned char* rawImageData = nullptr;
	if (!LoadImageData(relativePathToImageFile.string(), imgProps, rawImageData)) return false;
	out_texture = CreateFromData(rawImageData, imgProps, relativePathToImageFile, assetId, isInternal);
	return true;
}

bool Texture::LoadAndBind(const std::string& relativePathToImageFile, ImageProperties& out_imgProps, unsigned int out_textureId) {
	unsigned char* rawImageData = nullptr;
	if (!LoadImageData(relativePathToImageFile, out_imgProps, rawImageData)) return false;
	glGenTextures(1, &out_textureId);
	BindToGPUAndFreeData(textureId, out_imgProps, rawImageData);
	return true;
}

Texture* Texture::CreateFromData(unsigned char* rawImageData, const ImageProperties& imgProps, const std::filesystem::path& relativePathToImageFile, const ::AssetId& assetId, bool isInternal) {
	unsigned int textureId;
	glGenTextures(1, &textureId);
	BindToGPUAndFreeData(textureId, imgProps, rawImageData);
	std::cout << "Image " << relativePathToImageFile.string().c_str() << " bound to textureID: " << textureId << std::endl;
	return new Texture(textureId, relativePathToImageFile, imgProps, assetId, isInternal);
}

void Texture::RefreshFromDataAndFree(unsigned char* rawImageData, const ImageProperties& imgProps) {
	BindToGPUAndFreeData(textureId, imgProps, rawImageData);
	std::cout << "Image " << Name << " refreshed to textureID: " << textureId << std::endl;
}

bool Texture::CreateNew(const std::filesystem::path& relativePathToImageFile, bool isInternal, bool isPartOfTextureSheet, Texture*& out_texture, AssetHeader& out_assetHeader) {
	if (!Create(relativePathToImageFile, out_texture, isInternal, AssetId::CreateNewAssetId())) {
		return false;
	}
	out_assetHeader.aType = isInternal ? AssetType::TextureInternal : AssetType::Texture;
	out_assetHeader.aId = out_texture->AssetId;
	//TODO: relativeAssetPath is wrong here
	out_assetHeader.relativeAssetPath = relativePathToImageFile;
	if (!isPartOfTextureSheet) {
		//don't save meta file information when its a tileSheet
		out_assetHeader.relativeAssetPath = out_texture->GetRelativeAssetPath();
		out_texture->SaveToFile();
	}
	return true;
}

void Texture::SliceSubTextureFromData(unsigned char* rawImageData, const ImageProperties& imProps, const SubTextureData& subTextureData, Texture*& out_TexturePtr) const {
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

	//if asset id exists, refresh it instead of creating a new one
	if (Resources::AssetIsLoaded(subTextureData.assetId)) {
		Texture* t = nullptr;
		if (!Resources::TryGetTexture(AssetId, t)) throw std::exception("unable to get loaded texture");
		t->RefreshFromDataAndFree(rawSubTextureData, subTexImgProps);
		return;
	}

	out_TexturePtr = CreateFromData(rawSubTextureData, subTexImgProps, GetImageFilePath(), subTextureData.assetId, false);
}

bool Texture::CreateSubTextures(const std::vector<SubTextureData>& subTextureData, std::vector<Texture*>& out_textures) const {
	if (this == empty) {
		std::cout << "Tried to slice empty texture" << std::endl;
		return false;
	}

	unsigned char* rawImageData = nullptr;
	ImageProperties imageProps{};
	//load without flip so yOffset is from the top instead of the bottom as expected
	if (!LoadImageData(GetImageFilePath(), imageProps, rawImageData, false)) return false;

	for (const auto& sData : subTextureData) {
		if (imageProps.width < sData.xOffset + sData.width || imageProps.height < sData.yOffset + sData.height) {
			std::cout << "ERROR: Can't create SubTexture from " << GetRelativeAssetPath() << " :" << "SubTexture does not fit within Texture!" << std::endl;
			free(rawImageData);
			return false;
		}
	}

	const std::filesystem::path p = GetRelativeAssetPath();
	const std::filesystem::path parentPath = p.parent_path();

	int subTextureCount = 0;
	for (const auto& sData : subTextureData) {
		Texture* tPtr = nullptr;
		const std::string fileName = p.stem().string() + subTextureSuffix + std::to_string(subTextureCount++) + p.extension().string();
		SliceSubTextureFromData(rawImageData, imageProps, sData, tPtr);
		out_textures.push_back(tPtr);
		//std::cout << "SubTextureData " << newPath.string().c_str() << " bound to textureID: " << tPtr->GetTextureID() << std::endl;
	}

	//stbi_write_png(newPath.string().c_str(), imageProperties.width, imageProperties.height, imageProperties.channelCount, subTextureData, static_cast<int>(newRowByteSize));

	free(rawImageData);
	return true;
}

//NOTE: seems to work with either relative or absolute paths
bool Texture::CanCreateFromPath(const char* absolutePath) {
	int _;
	const int ok = stbi_info(absolutePath, &_, &_, &_);
	return ok;
}

void Texture::Serialize(std::ostream& oStream) const {
	Serialization::writeToStream(oStream, isInternalTexture);
	Serialization::Serialize(oStream, GetImageFilePath());
}

bool Texture::Deserialize(std::istream& iStream, const AssetHeader& header, Texture*& out_texture) {
	bool isInternal = false; Serialization::readFromStream(iStream, isInternal);
	const std::string imageFilePath = Serialization::DeserializeStdString(iStream);
	return Create(imageFilePath, out_texture, isInternal, header.aId);
}

bool Texture::Refresh() {
	if (this == empty) {
		std::cout << "Tried to refresh empty texture" << std::endl;
		return false;
	}
	unsigned char* rawImageData = nullptr;
	if (!LoadImageData(GetImageFilePath(), imageProperties, rawImageData)) return false;
	RefreshFromDataAndFree(rawImageData, imageProperties);

	return true;
}

Texture::~Texture() {
	glDeleteTextures(1, &textureId);
	std::cout << "Image " << Name << " deleted." << std::endl;
}
