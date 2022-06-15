#pragma once
#include "Input.h"

namespace GridTools {

	class GridTool {
	public:
		virtual ~GridTool() = default;
		GridToolBar* toolBar;

		virtual void OnSelect() = 0;
		virtual void OnDeselect() = 0;
		virtual void OnInteract(const InputMouseEvent* event, const glm::ivec2& position, bool& isStale) = 0;
	};

	class PlacerTool : public GridTool {
	public:
		void OnSelect() override { }
		void OnDeselect() override { }
		void OnInteract(const InputMouseEvent* event, const glm::ivec2& position, bool& isStale) override;
	};

	class EraserTool : public GridTool {
	public:
		void OnSelect() override {}
		void OnDeselect() override {}
		void OnInteract(const InputMouseEvent* event, const glm::ivec2& position, bool& isStale) override;
	};
}

