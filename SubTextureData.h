#pragma once
#include "Serialization.h"


namespace Rendering {
	struct SubTextureData {
		int xOffset;
		int yOffset;
		int width;
		int height;

		SubTextureData(int xOffset, int yOffset, int width, int height);
	};
}

namespace Serialization {
	inline std::ostream& Serialize(std::ostream& oStream, const Rendering::SubTextureData& subTextureData) {
		writeToStream(oStream, subTextureData.xOffset);
		writeToStream(oStream, subTextureData.yOffset);
		writeToStream(oStream, subTextureData.width);
		writeToStream(oStream, subTextureData.height);
		return oStream;
	}

	inline Rendering::SubTextureData DeserializeSubTextureData(std::istream& stream) {
		Rendering::SubTextureData subTextureData{0, 0, 0, 0};
		readFromStream(stream, subTextureData.xOffset);
		readFromStream(stream, subTextureData.yOffset);
		readFromStream(stream, subTextureData.width);
		readFromStream(stream, subTextureData.height);

		return subTextureData;
	}
}

