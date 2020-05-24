#ifndef COMMON_GUI_H
#define COMMON_GUI_H

#include <ncine/imgui.h>
#include <nctl/String.h>

namespace nc = ncine;

namespace ui {

static const unsigned int MaxStringLength = 256;

extern nctl::String comboString;
extern nctl::String auxString;

#ifdef __ANDROID__
extern nctl::String androidSaveDir;
#endif

int inputTextCallback(ImGuiInputTextCallbackData *data);
};

#endif
