#pragma once
#include <memory>
#include <string>
#include "Serialization.h"

struct AssetId_private;
//wraps GUID
class AssetId {
	std::shared_ptr<AssetId_private> assetId_privateSPtr;
public:
	AssetId();
	static AssetId CreateNewAssetId();
	static bool TryParse(std::string& string, AssetId& out_assetId);

	bool operator==(const AssetId& other) const;
	operator std::string() const;
	std::string ToString() const;
	unsigned short GetHashCode() const;
	bool IsEmpty() const;
};

namespace std {
	template<>
	struct std::hash<AssetId> {
		std::size_t operator()(const AssetId& key) const noexcept {
			return key.GetHashCode();
		}
	};
}

namespace Serialization {
	inline std::ostream& Serialize(std::ostream& oStream, const AssetId& assetId) {
		return Serialize(oStream, assetId.ToString());
	}

	inline bool TryDeserializeAssetId(std::istream& stream, AssetId& out_AssetId) {
		auto str = DeserializeStdString(stream);
		return AssetId::TryParse(str, out_AssetId);
	}
}