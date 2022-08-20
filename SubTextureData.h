#pragma once
#include "AssetId.h"
#include "Serialization.h"


namespace Rendering {
	struct SubTextureData {
		int xOffset = 0;
		int yOffset = 0;
		int width = 0;
		int height = 0;
		AssetId assetId;

		SubTextureData(int xOffset, int yOffset, int width, int height);
		SubTextureData(int xOffset, int yOffset, int width, int height, AssetId assetId);
		SubTextureData() : SubTextureData(0, 0, 16, 16) {}
	};
}

namespace Serialization {
	inline std::ostream& Serialize(std::ostream& oStream, const Rendering::SubTextureData& subTextureData) {
		Serialize(oStream, subTextureData.assetId);
		writeToStream(oStream, subTextureData.xOffset);
		writeToStream(oStream, subTextureData.yOffset);
		writeToStream(oStream, subTextureData.width);
		writeToStream(oStream, subTextureData.height);
		return oStream;
	}

	inline Rendering::SubTextureData DeserializeSubTextureData(std::istream& stream) {
		Rendering::SubTextureData subTextureData{0, 0, 0, 0, {}};
		TryDeserializeAssetId(stream, subTextureData.assetId);
		readFromStream(stream, subTextureData.xOffset);
		readFromStream(stream, subTextureData.yOffset);
		readFromStream(stream, subTextureData.width);
		readFromStream(stream, subTextureData.height);

		return subTextureData;
	}
}

