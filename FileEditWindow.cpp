#include "FileEditWindow.h"

#include <sstream>

#include "Tile.h"
#include "ImGuiHelper.h"

template class FileEditWindow<Tiles::Tile>;
//template <>
//FileEditWindow<Tiles::Tile>::FileEditWindow(Tiles::Tile* data, std::unique_ptr<Tiles::Tile> tempCopy) {
//	
//}
template <class T>
bool FileEditWindow<T>::RenderImGui() {
	using namespace ImGui;
	bool showWindow = true;
	std::ostringstream oss;
	oss << (void*)this;
	std::string s(oss.str());
	std::string name = "File Properties##" + s;
	if (Begin(name.c_str(), &showWindow)) {
		if(EditFile()) showWindow = false;
	}
	End();

	return showWindow;
}

template<>
bool FileEditWindow<Tiles::Tile>::EditFile() {
	return fileData->ImGuiEditTile(temporaryFileCopy.get());
}

template <class T>
FileEditWindow<T>::FileEditWindow(T* data, std::unique_ptr<T> tempCopy) : fileData(data), temporaryFileCopy(std::move(tempCopy)) {
}

template <class T>
void FileEditWindow<T>::NewEditWindow(T* data) {
	std::unique_ptr<T> tempCopy = std::make_unique<T>(*data);
	activeWindows.emplace_back(new FileEditWindow<T>(data, std::move(tempCopy)));
}

template <class T>
void FileEditWindow<T>::NewFileCreationWindow() {
	//When creating a new file, the FileEditWindow owns it and the temporary file pointer is the same object as the file being edited
	//both will be freed when window closes and have to be loaded from memory again
	//std::unique_ptr<T> fileDataUPTR = std::make_unique<T>();
	//auto windowUPTR = std::make_unique<FileEditWindow>(fileDataUPTR.get(), fileDataUPTR);
	//activeWindows.emplace_back(windowUPTR);
}

template <class T>
void FileEditWindow<T>::Close() {
	auto findWindow = [this] (const std::unique_ptr<IFileEditWindow>& item) -> bool {
		return item.get() == this;
	};
	const auto result = std::find_if(activeWindows.begin(), activeWindows.end(), findWindow);
	if (result == activeWindows.end())
		std::cout << "unable to find active window" << std::endl;
	else
		activeWindows.erase(result);

}
