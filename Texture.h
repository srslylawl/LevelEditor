#pragma once
#include <string>

class Texture {
public:
	Texture() = default;

	Texture(unsigned id, const std::string& path, int width, int height, int channel_count)
		: ID(id),
		  path(path),
		  width(width),
		  height(height),
		  channelCount(channel_count) {}

	unsigned int ID = -1;
	std::string path;
	int width;
	int height;
	int channelCount;
};

