#ifndef COMMON_GUI_H
#define COMMON_GUI_H

#include <ncine/imgui.h>
#include <nctl/String.h>

namespace nc = ncine;

namespace ncine {
class DropEvent;
}

namespace ui {

static const unsigned int MaxStringLength = 256;

extern nctl::String comboString;
extern nctl::String auxString;

#ifdef __ANDROID__
extern nctl::String androidCfgDir;
extern nctl::String androidSaveDir;
#endif

extern nctl::String projectsDataDir;
extern nctl::String texturesDataDir;
extern nctl::String scriptsDataDir;

int inputTextCallback(ImGuiInputTextCallbackData *data);

extern const nc::DropEvent *dropEvent;
extern unsigned int dropUpdateFrames;
}

#endif
