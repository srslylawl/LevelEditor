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

	static const char* GetFileExtension(const AssetType aType) {
		switch (aType) {
			case AssetType::UNKNOWN:
			case AssetType::Texture:
			case AssetType::TextureInternal:
			case AssetType::TextureSheet:
				return ".asset";
			case AssetType::Tile:
				return ".tile";
			case AssetType::Level:
				return ".level";
			default: throw std::exception("unhandled default case");
		}
	}

	static bool IsAssetFileExtension(const std::string& extension) {
		return extension == ".asset" || extension == ".tile" || extension == ".level";
	}

	static bool TryReadHeaderFromFile(const std::filesystem::path& absolutePath, AssetHeader* out_header) {
		if (!absolutePath.has_extension() || !IsAssetFileExtension(absolutePath.extension().string())) return false;
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
			std::cerr << "unable to deserialize header for " << absolutePath << std::endl;
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
	std::filesystem::path ParentPath;

	//Deduces ParentPath and Name from AssetPath
	PersistentAsset(::AssetId assetId, const ::AssetType type, const std::filesystem::path assetPath) :
		Serialization::Serializable<T>(assetPath.filename().string()), AssetId(assetId), AssetType(type), ParentPath(assetPath.parent_path()) {
	}

	PersistentAsset(::AssetId assetId, const ::AssetType type, const std::filesystem::path parentPath, const std::string name) :
		Serialization::Serializable<T>(name), AssetId(assetId), AssetType(type), ParentPath(parentPath) {
	}

	virtual std::filesystem::path GetRelativeAssetPath() const {
		std::filesystem::path p = ParentPath;
		p.append(Serialization::Serializable<T>::Name + AssetHeader::GetFileExtension(AssetType));
		return p;
	}

	virtual bool CanSave(std::string& out_errorMsg, bool allowOverwrite = true) const {
		if (AssetId.IsEmpty()) {
			out_errorMsg = "AssetId is empty.";
			return false;
		}
		if (Serialization::Serializable<T>::Name.empty()) {
			out_errorMsg = "Name is missing.";
			return false;
		}
		std::error_code error;
		bool fileNameAlreadyExists = std::filesystem::exists(GetRelativeAssetPath(), error);
		if (fileNameAlreadyExists && !allowOverwrite) {
			out_errorMsg = "A file with that name already exists.";
			return false;
		}
		if (error.value() != 0) {
			out_errorMsg = error.message();
			return false;
		}
		if (ParentPath.empty()) {
			out_errorMsg = "Missing Parent Path";
			return false;
		}

		return true;
	}

	void SaveToFile() const {
		SaveToFile(GetRelativeAssetPath());
	}

	void SaveToFile(const std::filesystem::path& filePath) const {
		std::string msg;
		if (!CanSave(msg)) {
			std::cout << "Unable to save: " << filePath << " " << msg << std::endl;
			return;
		}

		if (!std::filesystem::exists(filePath.parent_path())) {
			std::filesystem::create_directory(filePath.parent_path());
		}
		//std::cout << "Writing path: " << filePath << std::endl;
		std::ofstream stream(filePath, std::iostream::binary);
		AssetHeader::Write(stream, this);
		Serialize(stream);
		stream.close();
		std::cout << "Saved: " << filePath << "id: " << AssetId.ToString() << std::endl;
	}

	// Loads a Serializable Class from File and allocates on Free Store (has to be deleted manually) -- note: replace with unique_ptr for ownership clarity?
	static bool LoadFromFile(const char* relativePathToFile, T*& out) {
		//std::cout << "Reading path: " << relativePathToFile << std::endl;
		std::ifstream file(Files::GetAbsolutePath(relativePathToFile), std::iostream::binary);
		if (!file) return false; // Check for error

		AssetHeader header;
		if (!AssetHeader::Read(file, &header)) {
			std::cout << "Unable to read header of file: " << relativePathToFile << std::endl;
			file.close();
			return false;
		}
		header.relativeAssetPath = relativePathToFile;
		bool success = T::Deserialize(file, header, out);
		file.close();

		return success && out != nullptr;
	}

	void Rename(const std::string& new_Path) const {
		const std::filesystem::path newPath = new_Path;
		if (!exists(newPath.parent_path())) {
			create_directory(newPath.parent_path());
		}
		std::filesystem::rename(GetRelativeAssetPath(), newPath);
	}


};

