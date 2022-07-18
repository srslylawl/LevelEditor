#pragma once
#include "FileBrowserFile.h"

class IFileEditWindow {
public:
	virtual ~IFileEditWindow() = default;
	inline static std::vector<std::unique_ptr<IFileEditWindow>> activeWindows;
	inline static std::vector<std::unique_ptr<IFileEditWindow>*> windowsToClose;

	virtual bool RenderImGui() = 0;
	virtual void Close() = 0;

	static void RenderAll() {
		for (auto& windowUPTR : activeWindows)
			if (!windowUPTR->RenderImGui()) {
				//std::unique_ptr<IFileEditWindow>* uptr = &windowUPTR;
				windowsToClose.emplace_back(&windowUPTR);
			}

		if(windowsToClose.empty()) return;

		for (auto& element : windowsToClose) {
			element->get()->Close();
		}

		windowsToClose.clear();
	}
};

template<class T>
class FileEditWindow : public IFileEditWindow {
	T* fileData = nullptr;
	std::unique_ptr<T> temporaryFileCopy;


	bool RenderImGui() override;
	bool EditFile();
	FileEditWindow(T* data, std::unique_ptr<T> tempCopy);
public:
	//Holds reference to file and will create a temporary copy that can be edited while the window is open.
	static void NewEditWindow(T* dataToEdit);
	static void NewFileCreationWindow();

	void Close() override;

};




