#include "gui/gui_common.h"
#include <ncine/IFile.h>

namespace ui {

int inputTextCallback(ImGuiInputTextCallbackData *data)
{
	nctl::String *string = reinterpret_cast<nctl::String *>(data->UserData);
	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
	{
		// Resize string callback
		ASSERT(data->Buf == string->data());
		string->setLength(static_cast<unsigned int>(data->BufTextLen));
		data->Buf = string->data();
	}
	return 0;
}

nctl::String joinPath(const nctl::String &first, const nctl::String &second)
{
	const char *pathSeparator = (first.isEmpty() ==false && first[first.length() - 1] != '/') ? "/" : "";
	return nctl::String(first + pathSeparator + second);
}

bool checkPathOrConcatenate(const nctl::String &pathToConcatenate, const nctl::String &pathToCheck, nctl::String &destString)
{
	// Giving priority to the concatenated path
	destString = ui::joinPath(pathToConcatenate, pathToCheck);
	if (nc::IFile::access(destString.data(), nc::IFile::AccessMode::READABLE))
		return true;
	else if (nc::IFile::access(pathToCheck.data(), nc::IFile::AccessMode::READABLE))
	{
		destString = pathToCheck;
		return true;
	}
	else
		return false;
}

}
