#pragma once
#include <filesystem>
#include <string>

using namespace std;

class Files {
	static const constexpr char* days[] = { "Sprites" };

public:
	static bool VerifyDirectory(const char* directory, bool createIfNotExists = true) {
		auto path = filesystem::current_path().append(directory);

		if (filesystem::is_directory(path)) {
			return true;
		}

		if (createIfNotExists) {
			filesystem::create_directory(path);
			return true;
		}

		return false;
	}

};

