#pragma once
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION //required by stb lib
#include "stb_image.h"

using namespace std;

inline bool VerifyDirectory(const char* directory, bool createIfNotExists = true) {
	const filesystem::path path = filesystem::current_path().append(directory);

	if (filesystem::is_directory(path)) {	
		return true;
	}

	if (createIfNotExists) {
		filesystem::create_directory(path);
		return true;
	}

	return false;
}

inline bool LoadImage(const char* path, unsigned char* OUT_data, int* OUT_width, int* OUT_height, int* OUT_channelCount) {
	int width, height, channelCount;
	OUT_data = stbi_load(path, &width, &height, &channelCount, 0);

	return OUT_data != nullptr;
}