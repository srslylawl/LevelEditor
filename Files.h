#pragma once
#include <filesystem>
#include <fstream>
#include "stb_image.h"
#include <iostream>


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
	inline bool IsSupportedImage(const char* path) {
		int x,y,n;
		const int ok = stbi_info(path, &x, &y, &n);
		return ok;
	}

	inline bool LoadImageFile(const char* path, unsigned char*& OUT_data, int* OUT_width, int* OUT_height, int* OUT_channelCount) {
		stbi_set_flip_vertically_on_load(true);
		OUT_data = stbi_load(path, OUT_width, OUT_height, OUT_channelCount, 0);
		const bool success = OUT_data != nullptr;
		if (!success) std::cout << "Unable to load image: " << path << " : " << stbi_failure_reason() << std::endl;

		return success;
	}

	template<typename Serializable>
	void SaveToFile(const char* fileName, Serializable* serializable) {
		//TODO: make secure
		ofstream o(fileName, iostream::binary);
		serializable->Serialize(o);
		o.close();
	}

	template<typename Serializable>
	Serializable* LoadFromFile(const char* fileName) {
		//TODO: secure this
		ifstream file(fileName);
		Serializable* s = Serializable::Deserialize(file);
		file.close();

		return s;
	}

	bool OpenFileDialog(std::string& filePath, const char* filter);

}
