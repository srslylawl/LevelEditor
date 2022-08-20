#pragma once

class InputMouseEvent;
namespace GridTools {

	class GridTool {
	public:
		virtual ~GridTool() = default;
		GridToolBar* toolBar;

		GridTool(GridToolBar* toolBar) : toolBar(toolBar) {}
		virtual void OnSelect() = 0;
		virtual void OnDeselect() = 0;

		//Returns whether interaction was successful
		virtual bool OnInteract(const InputMouseEvent* event, const glm::ivec2& position, bool& isStale) = 0;
	};

	class PlacerTool : public GridTool {
	public:
		explicit PlacerTool(GridToolBar* gridToolBar) : GridTool(gridToolBar) {}

		void OnSelect() override { }
		void OnDeselect() override { }
		bool OnInteract(const InputMouseEvent* event, const glm::ivec2& position, bool& isStale) override;
	};

	class EraserTool : public GridTool {
	public:
		explicit EraserTool(GridToolBar* gridToolBar) : GridTool(gridToolBar) {}

		void OnSelect() override {}
		void OnDeselect() override {}
		bool OnInteract(const InputMouseEvent* event, const glm::ivec2& position, bool& isStale) override;
	};

	class SelectTool : public GridTool {
	public:
		explicit SelectTool(GridToolBar* gridToolBar) : GridTool(gridToolBar) {}
		void OnSelect() override {}
		void OnDeselect() override {}
		bool OnInteract(const InputMouseEvent* event, const glm::ivec2& position, bool& isStale) override;
	};
}

