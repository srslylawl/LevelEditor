#pragma once

#include <functional>

#include "FileBrowserFile.h"

class IFileEditWindow {
public:
	virtual ~IFileEditWindow() = default;
	inline static std::vector<std::unique_ptr<IFileEditWindow>> activeWindows;

	virtual bool RenderImGui() = 0;

	bool setFocus = false;
	std::function<void()> onClose = nullptr;

	static void RenderAll() {
		for (auto it = activeWindows.begin(); it != activeWindows.end();) {
			if (!it->get()->RenderImGui()) {
				if (it->get()->onClose != nullptr) {
					it->get()->onClose();
				}
				it = activeWindows.erase(it);
				continue;
			}
			++it;
		}
	}
};

template<class T>
class FileEditWindow : public IFileEditWindow {
	T* fileData = nullptr;
	std::unique_ptr<T> temporaryFileCopy;


	bool RenderImGui() override;
	bool EditFile();
public:
	FileEditWindow(std::unique_ptr<T> tempCopy, std::function<void()> onClose = nullptr);
	FileEditWindow(T* data, std::function<void()> onClose = nullptr);
	//Holds reference to file and will create a temporary copy that can be edited while the window is open.
	static void NewEditWindow(T* data, const std::function<void()> onClose = nullptr);
	static void NewFileCreationWindow(const std::function<void()> onClose = nullptr);
};




