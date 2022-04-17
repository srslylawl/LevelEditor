#pragma once
#include <map>

//#include "Files.h"
#include "Files.h"
#include "Texture.h"


class Resources {
public:
	inline static std::map<std::string, Texture> Textures;
	static void LoadTexture(std::string path, bool refresh = false);

};

