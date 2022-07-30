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
	bool IsSupportedImageFormat(const char* absolutePath);

	inline std::filesystem::path GetAbsolutePath(const std::string& path) {
		return std::filesystem::current_path().append(path);
	}

	inline std::string GetRelativePath(const std::string& absolutePath) {
		std::string result = absolutePath;
		const std::string currentPath = std::filesystem::current_path().lexically_normal().string() + "\\";
		const auto subStringPos = result.find(currentPath);
		if (subStringPos != std::string::npos)
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

	inline void ForEachInDirectory(const char* directory, const function<void(const char*)>& function, bool skipFolders = true) {
		for (auto& entry : GetDirectoryIterator(directory)) {
			if(entry.is_directory() && skipFolders) continue;
			string relativePath = directory;
			relativePath += "\\" + entry.path().filename().string();
			function(relativePath.c_str());
		}
	}

	template<typename Serializable>
	std::string GetRelativePathTo(Serializable* serializable) {
		return serializable->ParentDirectory + "\\" + serializable->Name + serializable->FileEnding;
	}

	template<typename Serializable>
	std::string GetRelativePathTo(Serializable* serializable, std::string nameOverride) {
		return serializable->ParentDirectory + "\\" + nameOverride + serializable->FileEnding;
	}


	template<typename Serializable>
	void SaveToFile(Serializable* serializable) {
		const std::filesystem::path filePath = GetRelativePathTo(serializable);
		if (!exists(filePath.parent_path())) {
			create_directory(filePath.parent_path());
		}

		ofstream o(filePath, iostream::binary);
		serializable->Serialize(o);
		o.close();
	}

	// Loads a Serializable Class from File and allocates on Free Store (has to be deleted manually)
	template<typename Serializable>
	bool LoadFromFile(const char* relativePathToFile, Serializable*& out) {
		//TODO: secure this
		ifstream file(GetAbsolutePath(relativePathToFile));

		if (!file) return false; // Check for error

		bool success = Serializable::Deserialize(file, out);
		file.close();

		return success && out != nullptr;
	}

	template<typename Serializable>
	void RenameFile(Serializable* serializable, const std::string& new_name) {
		std::string oldPath = GetRelativePathTo(serializable);
		std::filesystem::path newPath = GetRelativePathTo(serializable, new_name);
		if (!exists(newPath.parent_path())) {
			create_directory(newPath.parent_path());
		}
		std::filesystem::rename(oldPath, newPath);
	}

	bool OpenFileDialog(std::string& filePath, const char* filter);

}
