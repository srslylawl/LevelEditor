#pragma once
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION //required by stb lib
#include "stb_image.h"


namespace Files {
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

	inline bool LoadImageFile(const char* path, unsigned char*& OUT_data, int* OUT_width, int* OUT_height, int* OUT_channelCount) {
		stbi_set_flip_vertically_on_load(true);
		OUT_data = stbi_load(path, OUT_width, OUT_height, OUT_channelCount, 0);

		return OUT_data != nullptr;
	}

}
