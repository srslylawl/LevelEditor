#pragma once
#include <string>

namespace Rendering {
	class Texture {
	public:
		Texture() = default;

		Texture(unsigned id, std::string name, std::string path, int width, int height, int channel_count)
			: ID(id),
			path(std::move(path)),
			name(std::move(name)),
			width(width),
			height(height),
			channelCount(channel_count) {}

		unsigned int ID = -1;
		std::string path;
		std::string name;
		int width;
		int height;
		int channelCount;
	};
}


