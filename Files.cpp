#include "Files.h"
#include <combaseapi.h>
#include <SDL_messagebox.h>

#include "Texture.h"


namespace Files {
	bool IsSupportedImageFormat(const char* absolutePath) {
		return Rendering::Texture::CanCreateFromPath(absolutePath);
	}

	bool OpenFileDialog(std::string& out_filePath, const char* filter) {
		std::string fileName;
		fileName.resize(1024);

		OPENFILENAMEA ofn;
		ZeroMemory(&fileName[0], sizeof(char)*1024);
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = nullptr;  // If you have a window to center over, put its HANDLE here
		ofn.lpstrFilter = filter; //"Image Files (JPG, PNG, TGA, BMP, PSD, GIF, HDR, PIC, PNM)\0*.jpeg;*.png;*.tga;*.bmp;*.psd;*.gif;*.hdr;*.pic;*.pnm\0\0";
		ofn.lpstrFile = &fileName[0];
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrTitle = "Select a File.";
		ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (!GetOpenFileNameA(&ofn)) {
			std::string buffer;
			buffer.resize(1024);
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

			buffer = "Unable to open file: " + error;
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", &buffer[0], nullptr);

			return false;
		}
		out_filePath = fileName;
		return true;
	}

}
