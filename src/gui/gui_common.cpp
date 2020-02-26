#include "gui/gui_common.h"

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
	const char *pathSeparator = (first[first.length() - 1] != '/') ? "/" : "";
	return nctl::String(first + pathSeparator + second);
}

}
