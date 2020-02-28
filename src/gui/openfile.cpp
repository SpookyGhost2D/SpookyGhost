#ifdef _WIN32
	#include <ncine/common_windefines.h>
	#include <windef.h>
	#include <WinBase.h>
	#include <winuser.h>
	#include <shellapi.h>
	#include <fileapi.h>
#endif

#include "gui/UserInterface.h"

void UserInterface::openFile(const char *filename)
{
#if defined(_WIN32)
	static char buffer[256];
	_fullpath(buffer, filename, 256);
	ShellExecute(NULL, "open", buffer, NULL, NULL, SW_SHOWNORMAL);
#elif defined(__APPLE__)
	static nctl::String execString(256);
	execString.format("open %s", filename);
	system(execString.data());
#elif defined(__linux__)
	static nctl::String execString(256);
	execString.format("xdg-open %s", filename);
	system(execString.data());
#endif
}
