#pragma once
#include <filesystem>

namespace Files {
	class File {
		std::string relativePath;
		std::string fileName;

	public:
		void SetFilePath(std::filesystem::path fullPath)
	};
}

