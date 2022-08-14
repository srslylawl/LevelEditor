#pragma once

#include <filesystem>
#include <fstream>
#include "AssetId.h"
#include "Files.h"

enum class AssetType {
	UNKNOWN = 0,
	TextureInternal = 1,
	Texture = 2,
	TextureSheet = 3,
	Tile = 4,
	Level = 5
};

class IPersistentAsset {
public:
	virtual ~IPersistentAsset() = default;
	virtual AssetType GetAssetType() const = 0;
	virtual AssetId GetAssetId() const = 0;

};

struct AssetHeader {
	static constexpr uint8_t Header = 1;
	static constexpr char FileExtension[] = ".asset";

	AssetType aType;
	//Always empty if not read from file
	std::filesystem::path relativeAssetPath;

	//Removes .asset extension essentially
	std::string GetCorrespondingFilePath() const {
		auto p = relativeAssetPath;
		p.replace_extension();
		return p.string();
	}

	AssetId aId;

	//AssetHeader can point to either a meta file or raw asset. Returns true if its a meta file.
	bool HasCorrespondingFile() const {
		switch (aType) {
			case AssetType::Texture:
			case AssetType::TextureInternal:
			case AssetType::TextureSheet: return true;
			case AssetType::Tile:
			case AssetType::Level: return false;
			case AssetType::UNKNOWN:
			default: throw std::exception("unhandled default case");
		}
	}

	static bool TryReadHeaderFromFile(const std::filesystem::path& absolutePath, AssetHeader* out_header) {
		if (!absolutePath.has_extension() || absolutePath.extension() != FileExtension) return false;
		out_header->relativeAssetPath = Files::GetRelativePath(absolutePath);
		std::ifstream file(absolutePath);
		if (!file) {
			std::cerr << "unable to open file stream for " << absolutePath << std::endl;
			file.close();
			return false;
		}

		const bool success = Read(file, out_header);
		file.close();
		if (!success) {
			std::cerr << "unable to deserialize header" << std::endl;
			return false;
		}

		return true;
	}

	static bool Read(std::istream& iStream, AssetHeader* out_header) {
		//Header
		uint8_t headerCheck = 0; Serialization::readFromStream(iStream, headerCheck);
		if (headerCheck != Header) return false;

		//AssetType
		uint8_t assetType = 0; Serialization::readFromStream(iStream, assetType);
		out_header->aType = static_cast<AssetType>(assetType);

		//AssetId
		const bool success = Serialization::TryDeserializeAssetId(iStream, out_header->aId);
		if (!success) return false;

		return true;
	}

	static void Write(std::ostream& oStream, const IPersistentAsset* asset) {
		//Header
		Serialization::writeToStream(oStream, Header);
		//AssetType
		auto assetType = static_cast<uint8_t>(asset->GetAssetType());
		Serialization::writeToStream(oStream, assetType);
		//AssetId
		Serialization::Serialize(oStream, asset->GetAssetId());
	}
};

template<class T>
class PersistentAsset : public Serialization::Serializable<T>, public IPersistentAsset {
public:
	using Serialization::Serializable<T>::Serialize;

	AssetId AssetId;
	AssetType AssetType;
	::AssetType GetAssetType() const override {
		return AssetType;
	}
	::AssetId GetAssetId() const override {
		return AssetId;
	}

	static const std::string GetFileEnding();
	static const std::string GetParentDirectory();

	PersistentAsset(::AssetId assetId, const ::AssetType type, const std::string& name) : Serialization::Serializable<T>(name), AssetId(assetId), AssetType(type) {
	}

	virtual std::string GetRelativePath() const {
		return GetParentDirectory() + "\\" + Serialization::Serializable<T>::Name + AssetHeader::FileExtension;
	}

	virtual std::string GetRelativePath(const std::string& nameOverride) const {
		return GetParentDirectory() + "\\" + nameOverride + AssetHeader::FileExtension;
	}

	void SaveToFile() const {
		SaveToFile(GetRelativePath());
	}

	void SaveToFile(const std::string& pathOverride) const {
		if (AssetId.IsEmpty()) {
			std::cout << "Error: tried to save asset with empty Id: " << Serialization::Serializable<T>::Name << std::endl;
			return;
		}
		if (Serialization::Serializable<T>::Name.empty()) {
			std::cout << "Error: tried to save asset with empty Name: " << pathOverride << std::endl;
			return;
		}

		const std::filesystem::path path = pathOverride;
		if (!std::filesystem::exists(path.parent_path())) {
			std::filesystem::create_directory(path.parent_path());
		}
		std::cout << "Writing path: " << pathOverride << std::endl;
		std::ofstream stream(path, std::iostream::binary);
		AssetHeader::Write(stream, this);
		Serialize(stream);
		stream.close();
		std::cout << "Saved: " << path << "id: " << AssetId.ToString() << std::endl;
	}

	// Loads a Serializable Class from File and allocates on Free Store (has to be deleted manually)
	static bool LoadFromFile(const char* relativePathToFile, T*& out) {
		std::cout << "Reading path: " << relativePathToFile << std::endl;
		std::ifstream file(Files::GetAbsolutePath(relativePathToFile), std::iostream::binary);
		if (!file) return false; // Check for error

		AssetHeader header;
		if (!AssetHeader::Read(file, &header)) {
			std::cout << "Unable to read header of file: " << relativePathToFile << std::endl;
			file.close();
			return false;
		}
		bool success = T::Deserialize(file, header, out);
		file.close();

		return success && out != nullptr;
	}

	void Rename(const std::string& new_name) const {
		const std::string oldPath = GetRelativePath();
		const std::filesystem::path newPath = GetRelativePath(new_name);
		if (!exists(newPath.parent_path())) {
			create_directory(newPath.parent_path());
		}
		std::filesystem::rename(oldPath, newPath);
	}


};

