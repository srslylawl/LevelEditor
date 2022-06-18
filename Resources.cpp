#include "Resources.h"

#include <iostream>
#include "glad.h"
#include <filesystem>
#define STB_IMAGE_IMPLEMENTATION //required by stb lib
#include "stb_image.h" //DONT REMOVE THIS

void Resources::LoadTexture(std::string path, bool refresh) {
	const bool exists = Textures.count(path);
	if (exists && !refresh) return; // Texture already loaded and we don't want to refresh it

	std::filesystem::path p = path;

	unsigned int textureID;

	if (exists) {
		textureID = Textures[path].ID;
	}
	else {
		glGenTextures(1, &textureID);
	}

	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, channelCount;
	unsigned char* imageData = nullptr;
	if (!Files::LoadImageFile(path.c_str(), imageData, &width, &height, &channelCount)) {
		return;
	}

	if(!exists) {
		const auto newTexture = Rendering::Texture(textureID, p.filename().string(), path, width, height, channelCount);;
		Textures[path] = newTexture;
	}

	int colorProfile = 0;
	switch (channelCount) {
	case 3:
		colorProfile = GL_RGB;
		break;
	case 4:
		colorProfile = GL_RGBA;
		break;
	default:
		std::cout << "Color Profile " << channelCount << " not recognized" << std::endl;
		break;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, colorProfile, width, height, 0, colorProfile, GL_UNSIGNED_BYTE, imageData);
	glGenerateMipmap(GL_TEXTURE_2D);

	std::cout << "Image bound to textureID: " << textureID << std::endl;

	free(imageData);
}

