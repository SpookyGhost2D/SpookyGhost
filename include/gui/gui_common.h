#ifndef COMMON_GUI_H
#define COMMON_GUI_H

#include <ncine/imgui.h>
#include <nctl/String.h>

namespace nc = ncine;

namespace ui {

static const unsigned int MaxStringLength = 256;

static nctl::String comboString = nctl::String(1024 * 2);
static nctl::String auxString = nctl::String(MaxStringLength);

int inputTextCallback(ImGuiInputTextCallbackData *data);
};

#endif
