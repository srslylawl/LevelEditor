#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "Serialization.h"

namespace Tiles {
	enum class SurroundingTileFlags {
		NONE = 0,
		UP = 1 << 0,
		UP_RIGHT = 1 << 1,
		RIGHT = 1 << 2,
		DOWN_RIGHT = 1 << 3,
		DOWN = 1 << 4,
		DOWN_LEFT = 1 << 5,
		LEFT = 1 << 6,
		UP_LEFT = 1 << 7
	};

	inline SurroundingTileFlags operator|(SurroundingTileFlags a, SurroundingTileFlags b) {
		return static_cast<SurroundingTileFlags>(static_cast<int>(a) | static_cast<int>(b));
	}

	enum class AutoTilePatternFlag {
		SOLO = 1,

		CENTER,
		OUTER_TOP_LEFT,
		OUTER_TOP_MID,
		OUTER_TOP_RIGHT,
		OUTER_MID_LEFT,
		OUTER_MID_RIGHT,
		OUTER_BOT_LEFT,
		OUTER_BOT_MID,
		OUTER_BOT_RIGHT,

		INNER_TOP_LEFT,
		INNER_TOP_RIGHT,
		INNER_BOT_LEFT,
		INNER_BOT_RIGHT,

		SINGLE_LEFT,
		SINGLE_RIGHT,
		SINGLE_TOP,
		SINGLE_BOT,

		HORIZONTAL_MIDDLE,
		VERTICAL_MIDDLE
	};

	enum class AutoWallPatternFlag {
		SOLO = 1,

		CENTER,
		LEFT,
		RIGHT
	};

	struct TextureVariant {
		std::string Texture;
		float ProbabilityModifier;

		TextureVariant(const std::string& texture, float probabilityModifier = 1) : Texture(texture), ProbabilityModifier(probabilityModifier) {
		}
	};

	struct TileSlot {
		std::vector<TextureVariant> TileSprites;
	};

	class ITilePattern {
	public:
		virtual ~ITilePattern() = default;
		virtual void AddTextureVariant(int patternFlag, TextureVariant variant) = 0;
		virtual void RenderDearImGui() = 0;
		virtual const TileSlot* GetTileSlot(const SurroundingTileFlags& mask) const = 0;
		virtual std::unique_ptr<ITilePattern> Clone() const = 0;
		virtual uint8_t GetTypeByte() const = 0;
	};

	class AutoTilePattern : public ITilePattern {
	public:
		std::unordered_map<AutoTilePatternFlag, TileSlot> TileSlots;

		//Won't allow empty texture variants.
		void AddTextureVariant(int flag, TextureVariant variant) override {
			if (variant.Texture.empty()) return;
			TileSlots[static_cast<AutoTilePatternFlag>(flag)].TileSprites.emplace_back(std::move(variant));
		}

		static AutoTilePatternFlag PatternFromSurroundingTiles(const SurroundingTileFlags& mask);

		const TileSlot* GetTileSlot(const SurroundingTileFlags& mask) const override;

		std::unique_ptr<ITilePattern> Clone() const override {
			return std::make_unique<AutoTilePattern>(*this);
		}

		void RenderDearImGui() override;

		uint8_t GetTypeByte() const override {
			return 2;
		}
	};

	class SimpleTilePattern : public ITilePattern {
	public:
		TileSlot tileSlot;
		void AddTextureVariant(int patternFlag, TextureVariant variant) override {
			if (variant.Texture.empty()) return;
			tileSlot.TileSprites.push_back(variant);
		}

		const TileSlot* GetTileSlot(const SurroundingTileFlags& mask) const override {
			return &tileSlot;
		}

		void RenderDearImGui() override;

		std::unique_ptr<ITilePattern> Clone() const override {
			return std::make_unique<SimpleTilePattern>(*this);
		}

		uint8_t GetTypeByte() const override {
			return 1;
		}
	};

	class AutoWallPattern : public ITilePattern {
	public:
		std::unordered_map<AutoWallPatternFlag, TileSlot> TileSlots;


		std::unique_ptr<ITilePattern> Clone() const override {
			return std::make_unique<AutoWallPattern>(*this);
		}

		void AddTextureVariant(int patternFlag, TextureVariant variant) override {
			if (variant.Texture.empty()) return;
			TileSlots[static_cast<AutoWallPatternFlag>(patternFlag)].TileSprites.emplace_back(std::move(variant));
		}
		void RenderDearImGui() override;
		const TileSlot* GetTileSlot(const SurroundingTileFlags& mask) const override;
		uint8_t GetTypeByte() const override {
			return 3;
		}
	};
}

namespace Serialization {

	inline std::ostream& Serialize(std::ostream& oStream, const Tiles::TileSlot& tileSlot) {
		writeToStream(oStream, tileSlot.TileSprites.size());
		for (const auto& TileSprite : tileSlot.TileSprites) {
			Serialize(oStream, TileSprite.Texture);
			writeToStream(oStream, TileSprite.ProbabilityModifier);
		}
		return oStream;
	}

	inline Tiles::TileSlot DeserializeTileSlot(std::istream& stream) {
		Tiles::TileSlot tileSlot;
		size_t count = 0; readFromStream(stream, count);

		for (size_t i = 0; i < count; ++i) {
			std::string texPath = DeserializeStdString(stream);
			float probability = 1; readFromStream(stream, probability);
			tileSlot.TileSprites.emplace_back(texPath, probability);
		}

		return tileSlot;
	}

	inline std::ostream& Serialize(std::ostream& oStream, const Tiles::AutoTilePattern& tilePattern) {
		writeToStream(oStream, tilePattern.TileSlots.size());

		for (const auto& [key, entry] : tilePattern.TileSlots) {
			writeToStream(oStream, static_cast<int>(key));
			Serialize(oStream, entry);
		}

		return oStream;
	}

	inline std::ostream& Serialize(std::ostream& oStream, const Tiles::SimpleTilePattern& tilePattern) {
		Serialize(oStream, tilePattern.tileSlot);

		return oStream;
	}

	inline std::ostream& Serialize(std::ostream& oStream, const Tiles::ITilePattern* iTilePattern) {
		const auto typeByte = iTilePattern->GetTypeByte();
		writeToStream(oStream, typeByte);
		switch (typeByte) {
			case 1:
			{
				const Tiles::SimpleTilePattern& pattern = *dynamic_cast<const Tiles::SimpleTilePattern*>(iTilePattern);
				Serialize(oStream, pattern);
			}
			break;
			case 2:
			{
				const Tiles::AutoTilePattern& pattern = *dynamic_cast<const Tiles::AutoTilePattern*>(iTilePattern);
				Serialize(oStream, pattern);
			}
			break;
			case 3:
				throw std::exception("serializing autowall not yet implemented");
				break;
			default:
				break;
		}

		return oStream;
	}

	inline Tiles::AutoTilePattern DeserializeAutoTilePattern(std::istream& stream) {
		Tiles::AutoTilePattern tilePattern;
		size_t count = 0; readFromStream(stream, count);
		for (size_t i = 0; i < count; ++i) {
			int pattern = 0; readFromStream(stream, pattern);
			tilePattern.TileSlots[static_cast<Tiles::AutoTilePatternFlag>(pattern)] = DeserializeTileSlot(stream);
		}

		return tilePattern;
	}

	inline Tiles::SimpleTilePattern DeserializeSimpleTilePattern(std::istream& stream) {
		Tiles::SimpleTilePattern tilePattern;
		tilePattern.tileSlot = DeserializeTileSlot(stream);

		return tilePattern;
	}

	inline std::unique_ptr<Tiles::ITilePattern> DeserializeITilePattern(std::istream& stream) {
		uint8_t typeByte;
		readFromStream(stream, typeByte);

		switch (typeByte) {
			case 1:
				//SimpleTile
				return DeserializeSimpleTilePattern(stream).Clone();
			case 2:
				//AutoTile
				return DeserializeAutoTilePattern(stream).Clone();
			case 3:
				//AutoWall
				throw std::exception("deserializing autowall not yet implemented");
			default:
				std::cout << "unable to deserialize ITilePattern, as typeByte was not recognized: " << typeByte << std::endl;
				return nullptr;
		}
	}
}

