#pragma once
#include <filesystem>
#include <fstream>
#include <functional>

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

	inline std::filesystem::path GetAbsolutePath(const std::string& path) {
		return std::filesystem::current_path().append(path);
	}

	inline auto GetDirectoryIterator(const char* directory) {
		return std::filesystem::directory_iterator(GetAbsolutePath(directory));
	}

	inline void ForEachInDirectory(const char* directory, const function<void(const char*)>& function) {
		for (auto& entry : GetDirectoryIterator(directory)) {
			string relativePath = directory;
			relativePath += "/" + entry.path().filename().string();
			function(relativePath.c_str());
		}
	}

	template<typename Serializable>
	void SaveToFile(const char* fileName, Serializable* serializable) {
		//TODO: make secure
		ofstream o(fileName, iostream::binary);
		serializable->Serialize(o);
		o.close();
	}

	// Loads a Serializable Class from File and allocates on Free Store (has to be deleted manually)
	template<typename Serializable>
	bool LoadFromFile(const char* fileName, Serializable*& out) {
		//TODO: secure this
		ifstream file(GetAbsolutePath(fileName));

		if(!file) return false; // Check for error

		out = Serializable::Deserialize(file);
		file.close();

		return out != nullptr;
	}

	bool OpenFileDialog(std::string& filePath, const char* filter);

}
