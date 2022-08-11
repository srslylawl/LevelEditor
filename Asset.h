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

struct AssetHeader {
	static constexpr uint8_t Header = 1;
	static constexpr char FileExtension[] = ".asset";

	AssetType aType;

	//Can be null if not read from File
	std::filesystem::path aPath;
	AssetId aId;

	//AssetHeader can point to either a meta file or raw asset. Returns true if its a meta file.
	bool HasCorrespondingFile() const {
		switch (aType) {
			case AssetType::Texture:
			case AssetType::TextureInternal:
			case AssetType::TextureSheet:
			case AssetType::Tile: return true;
			case AssetType::Level: return false;
			case AssetType::UNKNOWN:
			default: throw std::exception("unhandled default case");
		}
	}

	static bool ReadFromFile(const std::filesystem::path& absolutePath, AssetHeader* out_header) {
		if (!absolutePath.has_extension() || absolutePath.extension() != FileExtension) return false;
		std::ifstream file(absolutePath);
		if (!file) return false;

		const bool success = Deserialize(file, out_header);

		file.close();
		if(!success) return false;

		out_header->aPath = absolutePath;

		return true;
	}

	static bool Deserialize(std::istream& iStream, AssetHeader* out_header) {
		uint8_t headerCheck = 0; Serialization::readFromStream(iStream, headerCheck);
		if (headerCheck != Header) return false;


		uint8_t assetType = 0; Serialization::readFromStream(iStream, assetType);
		out_header->aType = static_cast<AssetType>(assetType);
		bool success = Serialization::TryDeserializeAssetId(iStream, out_header->aId);

		return success;
	}

	static void Write(std::ostream& oStream, AssetType type, AssetId id) {
		Serialization::writeToStream(oStream, Header);
		Serialization::writeToStream(oStream, static_cast<uint8_t>(type));
		Serialization::writeToStream(oStream, id);
	}
};

template<class SerializableT>
class Asset {
public:
	virtual ~Asset() = default;
	AssetId AssetId;
	std::string Name = "undefined";

	virtual void Serialize(std::ostream& oStream) const = 0;

	static bool Deserialize(std::istream& iStream, SerializableT*& out_T) {
		return SerializableT::Deserialize(iStream, out_T);
	}
};

template<class T>
class StandaloneAsset : public Asset<T> {
public:
	using Asset<T>::Serialize;
	static const std::string GetFileEnding();
	static const std::string GetParentDirectory();


	virtual std::string GetRelativePath() const {
		return GetParentDirectory() + "\\" + Asset<T>::Name + GetFileEnding() + AssetHeader::FileExtension;
	}

	virtual std::string GetRelativePath(const std::string& nameOverride) const {
		return GetParentDirectory() + "\\" + nameOverride + GetFileEnding() + AssetHeader::FileExtension;
	}

	void SaveToFile() const {
		SaveToFile(GetRelativePath());
	}

	void SaveToFile(const std::string& pathOverride) const {
		const std::filesystem::path filePath = pathOverride;
		if (!exists(filePath.parent_path())) {
			create_directory(filePath.parent_path());
		}

		std::ofstream o(filePath, std::iostream::binary);
		Serialize(o);
		o.close();
	}

	// Loads a Serializable Class from File and allocates on Free Store (has to be deleted manually)
	static bool LoadFromFile(const char* relativePathToFile, T*& out) {
		std::ifstream file(Files::GetAbsolutePath(relativePathToFile));

		if (!file) return false; // Check for error

		bool success = StandaloneAsset<T>::Deserialize(file, out);
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

