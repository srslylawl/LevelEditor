#pragma once
#include <filesystem>
#include <fstream>
#include <functional>
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
	bool IsSupportedImageFormat(const char* absolutePath);

	inline std::filesystem::path GetAbsolutePath(const std::string& path) {
		return std::filesystem::current_path().append(path);
	}

	inline std::string GetRelativePath(const std::string& absolutePath) {
		std::string result = absolutePath;
		const std::string currentPath = std::filesystem::current_path().lexically_normal().string() + "\\";
		const auto subStringPos = result.find(currentPath);
		if(subStringPos != std::string::npos)
			result.erase(subStringPos, currentPath.length());
		//std::cout << "GetRelativePath for " << absolutePath << ":" << std::endl;
		//std::cout << "CurrentPath: " << currentPath << std::endl;
		//std::cout << "Result: " << result << std::endl;
		return result;
	}

	inline std::string GetRelativePath(const std::filesystem::path& absolutePath) {
		return GetRelativePath(absolutePath.string());
	}

	inline auto GetDirectoryIterator(const char* directory) {
		return std::filesystem::directory_iterator(GetAbsolutePath(directory));
	}

	inline void ForEachInDirectory(const char* directory, const function<void(const char*)>& function) {
		for (auto& entry : GetDirectoryIterator(directory)) {
			string relativePath = directory;
			relativePath += "\\" + entry.path().filename().string();
			function(relativePath.c_str());
		}
	}

	template<typename Serializable>
	void SaveToFile(Serializable* serializable) {
		//TODO: make secure
		const std::string fileName = serializable->ParentDirectory + "\\" + serializable->Name + serializable->FileEnding;
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

		bool success = Serializable::Deserialize(file, out);
		file.close();

		return success && out != nullptr;
	}

	bool OpenFileDialog(std::string& filePath, const char* filter);

}
