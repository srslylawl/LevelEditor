#include "FileEditWindow.h"

#include <sstream>

#include "Tile.h"
#include "ImGuiHelper.h"

//template class FileEditWindow<Tiles::Tile>;


bool FileEditWindow::Edit() {
	using namespace ImGui;
	//Feed pointer address as id
	std::ostringstream oss;
	oss << static_cast<void*>(this);
	std::string s(oss.str());
	std::string name = "File Properties##" + s;

	if (setFocus) {
		SetNextWindowFocus();
		setFocus = false;
	}

	//Set to middle of screen and offset by amount of open windows so they don't overlap completely
	auto pos = GetMainViewport()->GetCenter();
	const auto windowCount = activeWindows.size();
	pos = ImVec2(pos.x + 10 * windowCount, pos.y + 10 * windowCount);
	SetNextWindowPos(pos, ImGuiCond_Once, ImVec2(0.5f, 0.5f));
	SetNextWindowSizeConstraints(ImVec2(250, 200), GetMainViewport()->Size);

	bool showWindow = true;
	if (Begin(name.c_str(), nullptr,  ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize)) {
		showWindow = !editable->RenderEditWindow(this, GetIsCreationWindow());
	}
	End();

	return showWindow;
}
FileEditWindow::FileEditWindow(IEditable* editable, std::function<void()> onClose) : editable(editable), OnClose(std::move(onClose)), oldPath(editable->IEditableGetAssetPath()) {}


void FileEditWindow::New(IEditable* editable, std::function<void()> onClose) {
	for (auto& activeWindowUPTR : activeWindows) {
		if (activeWindowUPTR->editable != editable) continue;

		//already exists, focus instead of creating new window
		activeWindowUPTR->setFocus = true;
		return;
	}
	activeWindows.emplace_back(new FileEditWindow(editable, std::move(onClose)));
}
