#include "gui/gui_common.h"

namespace ui {

nctl::String comboString(1024 * 2);
nctl::String auxString(MaxStringLength);

#ifdef __ANDROID__
nctl::String androidSaveDir(MaxStringLength);
#endif

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

}
