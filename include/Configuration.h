#ifndef CLASS_CONFIGURATION
#define CLASS_CONFIGURATION

#include <nctl/String.h>
#include <ncine/Vector2.h>
#include "gui/gui_common.h"

namespace nc = ncine;

/// The configuration to be loaded or saved
struct Configuration
{
	int version = 1;
	int width = 1920;
	int height = 1080;
	bool fullscreen = false;
	bool resizable = true;

	bool withVSync = true;
	int frameLimit = 0;

	int canvasWidth = 256;
	int canvasHeight = 256;
	int saveFileMaxSize = 16 * 1024;

	nctl::String startupScriptName = nctl::String(ui::MaxStringLength);
	bool autoPlayOnStart = true;

	nctl::String scriptsPath = nctl::String(ui::MaxStringLength);
	nctl::String texturesPath = nctl::String(ui::MaxStringLength);
};

#endif
