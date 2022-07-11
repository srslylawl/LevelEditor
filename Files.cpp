#include "Files.h"
#include <combaseapi.h>
#include <SDL_messagebox.h>

#include "Texture.h"


namespace Files {


	bool IsSupportedImageFormat(const char* absolutePath) {
		return Rendering::Texture::CanCreateFromPath(absolutePath);
	}

	bool OpenFileDialog(std::string& filePath, const char* filter) {
		char filename[1024];

		OPENFILENAMEA ofn;
		ZeroMemory(&filename, sizeof(filename));
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = nullptr;  // If you have a window to center over, put its HANDLE here
		ofn.lpstrFilter = filter; //"Image Files (JPG, PNG, TGA, BMP, PSD, GIF, HDR, PIC, PNM)\0*.jpeg;*.png;*.tga;*.bmp;*.psd;*.gif;*.hdr;*.pic;*.pnm\0\0";
		ofn.lpstrFile = filename;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrTitle = "Select a File, yo!";
		ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (!GetOpenFileNameA(&ofn)) {
			char buff[1024];
			std::string error;
			switch (CommDlgExtendedError())
			{
			case CDERR_DIALOGFAILURE: error = "CDERR_DIALOGFAILURE";	break;
			case CDERR_FINDRESFAILURE: error = "CDERR_FINDRESFAILURE";  break;
			case CDERR_INITIALIZATION: error = "CDERR_INITIALIZATION";  break;
			case CDERR_LOADRESFAILURE: error = "CDERR_LOADRESFAILURE";  break;
			case CDERR_LOADSTRFAILURE: error = "CDERR_LOADSTRFAILURE";  break;
			case CDERR_LOCKRESFAILURE: error = "CDERR_LOCKRESFAILURE";  break;
			case CDERR_MEMALLOCFAILURE:error = "CDERR_MEMALLOCFAILURE"; break;
			case CDERR_MEMLOCKFAILURE: error = "CDERR_MEMLOCKFAILURE";  break;
			case CDERR_NOHINSTANCE: error = "CDERR_NOHINSTANCE";		break;
			case CDERR_NOHOOK: error = "CDERR_NOHOOK";					break;
			case CDERR_NOTEMPLATE: error = "CDERR_NOTEMPLATE";			break;
			case CDERR_STRUCTSIZE: error = "CDERR_STRUCTSIZE";			break;
			case FNERR_BUFFERTOOSMALL: error = "FNERR_BUFFERTOOSMALL";  break;
			case FNERR_INVALIDFILENAME: error = "FNERR_INVALIDFILENAME"; break;
			case FNERR_SUBCLASSFAILURE: error = "FNERR_SUBCLASSFAILURE"; break;
			default: return false;
			}

			sprintf_s(buff, "Unable to open file: %s", error.c_str());
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", buff, nullptr);

			return false;
		}
		filePath = filename;
		return true;
	}

}
