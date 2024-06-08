#include "FileDialog.h"
#include <windows.h>
#include <commdlg.h>
#include "raylib.h"

std::string FileDialog::OpenFile(const char* filter)
{
	OPENFILENAMEA ofn;
	CHAR szFile[260] = {0};
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = *static_cast<HWND*>(GetWindowHandle()); //glfw
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	if(GetOpenFileNameA(&ofn) == TRUE)
		return ofn.lpstrFile;
	return std::string();
}

std::string FileDialog::SaveFile(const char* filter)
{
	OPENFILENAMEA ofn;
	CHAR szFile[260] = {0};
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = *static_cast<HWND*>(GetWindowHandle()); //raylib
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	if(GetSaveFileNameA(&ofn) == TRUE)
		return ofn.lpstrFile;
	return std::string();
}