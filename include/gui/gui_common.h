#ifndef COMMON_GUI_H
#define COMMON_GUI_H

#include <ncine/imgui.h>
#include <nctl/String.h>

namespace nc = ncine;

namespace ui {
	static const unsigned int MaxStringLength = 256;

	static nctl::String comboString = nctl::String(1024 * 2);
	static nctl::String auxString = nctl::String(MaxStringLength);
	static nctl::String filePath = nctl::String(MaxStringLength);

	int inputTextCallback(ImGuiInputTextCallbackData *data);

	nctl::String joinPath(const nctl::String &first, const nctl::String &second);
	bool checkPathOrConcatenate(const nctl::String &pathToConcatenate, const nctl::String &pathToCheck, nctl::String &destString);
};

#endif
