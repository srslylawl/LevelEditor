#pragma once
#include <filesystem>
#include <fstream>

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
	bool IsSupportedImageFormat(const char* path);

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
