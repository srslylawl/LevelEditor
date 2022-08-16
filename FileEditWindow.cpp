#include "FileEditWindow.h"

#include <sstream>

#include "Tile.h"
#include "ImGuiHelper.h"

//template class FileEditWindow<Tiles::Tile>;

template <class T>
bool FileEditWindow<T>::RenderImGui() {
	using namespace ImGui;
	bool showWindow = true;
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
	if (Begin(name.c_str(), &showWindow, ImGuiWindowFlags_NoSavedSettings)) {
		if (EditFile()) showWindow = false;
	}
	End();

	return showWindow;
}

template<>
bool FileEditWindow<Tiles::Tile>::EditFile() {
	return fileData->ImGuiEditTile(temporaryFileCopy.get());
}

template <class T>
FileEditWindow<T>::FileEditWindow(std::unique_ptr<T> tempCopy, const std::function<void()> onClose) : IFileEditWindow(onClose), fileData(tempCopy.get()), temporaryFileCopy(std::move(tempCopy)) {
}

template <class T>
FileEditWindow<T>::FileEditWindow(T* data, const std::function<void()> onClose) : IFileEditWindow(onClose), fileData(data), temporaryFileCopy(std::make_unique<T>(*data)) {
}

template <class T>
void FileEditWindow<T>::NewEditWindow(T* data, const std::function<void()> onClose) {
	//check if it exists already to avoid duplicate windows
	for (auto& activeWindowUPTR : activeWindows) {
		auto* fileEditWindow = dynamic_cast<FileEditWindow<T>*>(activeWindowUPTR.get());
		if (!fileEditWindow) continue;
		if (fileEditWindow->fileData != data) continue;

		activeWindowUPTR->setFocus = true;
		return;
	}

	activeWindows.emplace_back(new FileEditWindow<T>(data, onClose));
}

template <class T>
void FileEditWindow<T>::NewFileCreationWindow(const std::filesystem::path& directory, const std::function<void()> onClose) {
	//When creating a new file, the FileEditWindow owns it and the temporary file pointer is the same object as the file being edited
	//both will be freed when window closes and have to be loaded from memory again
	std::unique_ptr<T> fileDataUPTR = std::make_unique<T>();
	if (auto res = static_cast<PersistentAsset<T>*>(fileDataUPTR.get())) {
		res->ParentPath = directory;
	}
	activeWindows.emplace_back(new FileEditWindow<T>(std::move(fileDataUPTR), onClose));
}


template void FileEditWindow<Tiles::Tile>::NewFileCreationWindow(const std::filesystem::path& directory, const std::function<void()> onClose);
template void FileEditWindow<Tiles::Tile>::NewEditWindow(Tiles::Tile* data, const std::function<void()> onClose);
