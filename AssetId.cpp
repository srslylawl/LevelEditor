#include "AssetId.h"

#include <array>
#include <iostream>

#include "rpc.h"

struct AssetId_private {
	GUID guid{};

	AssetId_private(const GUID guid) : guid(guid) {
		std::cout << "empty guid\n";
	}

	static AssetId_private Create() {
		AssetId_private result;
		auto hres = CoCreateGuid(&result.guid);
		if (hres != S_OK) {
			//no idea tbh
			std::cout << "unable to create GUID for whatever reason" << std::endl;
		}

		return result;
	}

	AssetId_private() = default;
};

AssetId::AssetId() : assetId_privateSPTR(std::make_shared<AssetId_private>()) {
}
AssetId AssetId::CreateNewAssetId() {
	AssetId result;
	result.assetId_privateSPTR = std::make_shared<AssetId_private>(AssetId_private::Create());

	return result;
}

std::string AssetId::ToString() const {
	auto& guid = assetId_privateSPTR->guid;

	char guidStr[39];
	sprintf_s(
		guidStr,
		"{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

	return guidStr;
}

AssetId::AssetId(const std::string& string) {
	GUID guid;
	sscanf_s(&string[0],
			 "{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
			 &guid.Data1, &guid.Data2, &guid.Data3,
			 &guid.Data4[0], &guid.Data4[1], &guid.Data4[2],
			 &guid.Data4[3], &guid.Data4[4], &guid.Data4[5],
			 &guid.Data4[6], &guid.Data4[7]);
	assetId_privateSPTR = std::make_shared<AssetId_private>(guid);
}

bool AssetId::operator==(const AssetId& other) const {
	return other.assetId_privateSPTR->guid == assetId_privateSPTR->guid;
}

//AssetId& AssetId::operator=(const AssetId& other) noexcept {
//	if (&other == this) return *this;
//
//	assetId_privateSPTR = other.assetId_privateSPTR;
//	return *this;
//}
//
//AssetId::AssetId(const AssetId& other) : assetId_privateSPTR(other.assetId_privateSPTR) { }

AssetId::operator std::string() const {
	return ToString();
}

unsigned short AssetId::GetHashCode() const {
	RPC_STATUS status = RPC_S_OK;
	UUID* uuid = &assetId_privateSPTR->guid;
	const auto res = UuidHash(uuid, &status);
	return res;
}
