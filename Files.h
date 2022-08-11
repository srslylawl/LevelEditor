#pragma once
#include <filesystem>
#include <fstream>
#include <functional>

#include "Asset.h"

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

	inline bool PathIsSubPathOf(const std::filesystem::path subPath, const std::filesystem::path& parentPath) {
		auto& currentPath = const_cast<std::filesystem::path&>(subPath);
		while (currentPath.has_parent_path() && currentPath.has_relative_path()) {
			if (currentPath == parentPath) return true;
			currentPath = currentPath.parent_path();
		}

		return false;
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

	inline auto GetDirectoryIteratorRecursive(const char* directory) {
		return std::filesystem::recursive_directory_iterator(GetAbsolutePath(directory));
	}


	inline void ForEachInDirectory(const char* directory, const function<void(const char*)>& function, bool skipFolders = true, bool recursive = false) {
		// stinky duplicate code
		if (recursive) {
			for (auto& entry : GetDirectoryIteratorRecursive(directory)) {
				if (entry.is_directory() && skipFolders) continue;
				string relativePath = directory;
				relativePath += "\\" + entry.path().filename().string();
				function(relativePath.c_str());
			}
		}
		else {
			for (auto& entry : GetDirectoryIterator(directory)) {
				if (entry.is_directory() && skipFolders) continue;
				string relativePath = directory;
				relativePath += "\\" + entry.path().filename().string();
				function(relativePath.c_str());
			}
		}
	}

	bool OpenFileDialog(std::string& filePath, const char* filter);

}
