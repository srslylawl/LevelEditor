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

	enum class PatternFlag {
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

	struct TextureVariant {
		std::string Texture;
		float ProbabilityModifier;

		TextureVariant(const std::string& texture, float probabilityModifier = 1) : Texture(texture), ProbabilityModifier(probabilityModifier) {
		}
	};

	struct TileSlot {
		std::vector<TextureVariant> TileSprites;
	};

	class TilePattern {
	public:
		std::unordered_map<PatternFlag, TileSlot> TileSlots;

		//Won't allow empty texture variants.
		void AddTextureVariant(PatternFlag flag, TextureVariant variant) {
			if (variant.Texture.empty()) return;
			TileSlots[flag].TileSprites.emplace_back(std::move(variant));
		}

		static PatternFlag PatternFromSurroundingTiles(const SurroundingTileFlags& mask);

		const TileSlot* GetTileSlot(const SurroundingTileFlags& mask) const;

		void DearImGuiEditPattern();
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

	inline std::ostream& Serialize(std::ostream& oStream, const Tiles::TilePattern& tilePattern) {
		writeToStream(oStream, tilePattern.TileSlots.size());

		for (const auto& [key, entry] : tilePattern.TileSlots) {
			writeToStream(oStream, static_cast<int>(key));
			Serialize(oStream, entry);
		}

		return oStream;
	}

	inline Tiles::TilePattern DeserializeTilePattern(std::istream& stream) {
		Tiles::TilePattern tilePattern;
		size_t count = 0; readFromStream(stream, count);
		for (size_t i = 0; i < count; ++i) {
			int pattern = 0; readFromStream(stream, pattern);
			tilePattern.TileSlots[static_cast<Tiles::PatternFlag>(pattern)] = DeserializeTileSlot(stream);
		}

		return tilePattern;
	}
}

