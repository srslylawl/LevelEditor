#pragma once
#include <memory>
#include <string>

struct AssetId_private;
//wraps GUID
class AssetId {
	std::shared_ptr<AssetId_private> assetId_privateSPTR;
public:
	AssetId();
	static AssetId CreateNewAssetId();
	std::string ToString() const;
	AssetId(const std::string& string);

	bool operator==(const AssetId& other) const;
	//AssetId& operator=(const AssetId& other) noexcept;
	//AssetId(const AssetId& other) = default;
	operator std::string() const;

	unsigned short GetHashCode() const;
};

namespace std {
	template<>
	struct std::hash<AssetId> {
		std::size_t operator()(const AssetId& key) const noexcept {
			return key.GetHashCode();
		}
	};
}