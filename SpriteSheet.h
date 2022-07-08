#pragma once
#include <unordered_map>


namespace Rendering {
	class Texture;
}

//Represents a Texture that holds multiple individual sprites
class SpriteSheet {
	int spriteSize = 32; // in Pixels

public:
	std::unordered_map<std::string, Rendering::Texture*> Sprites;
};

