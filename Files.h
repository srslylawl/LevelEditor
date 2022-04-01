#pragma once
#include <filesystem>

using namespace std;

bool VerifyDirectory(const char* directory, bool createIfNotExists = true) {
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
