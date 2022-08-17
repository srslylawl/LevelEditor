#pragma once

#include <filesystem>
#include <functional>
#include <utility>


class FileEditWindow;

class IEditable {
public:
	virtual ~IEditable() = default;
	//return true when done
	virtual bool RenderEditWindow(FileEditWindow* editWindow, bool isNewFile) = 0;

	virtual std::filesystem::path IEditableGetAssetPath() = 0;
};

class FileEditWindow {
protected:
	inline static std::vector<std::unique_ptr<FileEditWindow>> activeWindows;

	IEditable* editable = nullptr;
	bool setFocus = false;
	std::function<void()> OnClose = nullptr;

	virtual bool GetIsCreationWindow() { return false;}

	//returns false when window should be closed
	bool Edit();
public:
	virtual ~FileEditWindow() = default;
	std::filesystem::path oldPath;


	FileEditWindow(IEditable* editable, std::function<void()> onClose = nullptr);


	static void New(IEditable* editable, std::function<void()> onClose = nullptr);

	static void RenderAll() {
		for (auto it = activeWindows.begin(); it != activeWindows.end();) {
			if (bool stillEditing = it->get()->Edit()) {
				++it;
				continue;
			}
			if (it->get()->OnClose != nullptr) {
				it->get()->OnClose();
			}
			it = activeWindows.erase(it);
		}
	}
};

class FileCreationWindow : public FileEditWindow {
	std::unique_ptr<IEditable> editableUPtr;

	bool GetIsCreationWindow() override { return true; }

public:
	template<class creatableT>
	static void New(const std::filesystem::path& directory, std::function<void()> onClose = nullptr);

	FileCreationWindow(std::unique_ptr<IEditable> editableUPtr, std::function<void()> onClose) : FileEditWindow(editableUPtr.get(),
		std::move(onClose)), editableUPtr(std::move(editableUPtr)) {
	}
};

template <class creatableT>
void FileCreationWindow::New(const std::filesystem::path& directory, std::function<void()> onClose) {
	std::unique_ptr<IEditable> editable = creatableT::CreateNew(directory);
	activeWindows.emplace_back(new FileCreationWindow(std::move(editable), onClose));
}




