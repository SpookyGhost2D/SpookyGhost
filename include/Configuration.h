#ifndef CLASS_CONFIGURATION
#define CLASS_CONFIGURATION

#include <nctl/Array.h>
#include <nctl/String.h>
#include <ncine/Vector2.h>
#include <ncine/AppConfiguration.h>
#include "gui/gui_common.h"

namespace nc = ncine;

/// The configuration to be loaded or saved
struct Configuration
{
	const int version = 6;

	int width = 1280;
	int height = 720;
	float refreshRate = 0.0f; // Added in version 6
	int windowPositionX = nc::AppConfiguration::WindowPositionIgnore; // Added in version 6
	int windowPositionY = nc::AppConfiguration::WindowPositionIgnore; // Added in version 6
	bool fullScreen = false;
	bool resizable = true;

	bool withVSync = true;
	int frameLimit = 0;

	int canvasWidth = 256;
	int canvasHeight = 256;

	bool autoGuiScaling = true; // Added in version 6
#ifdef __ANDROID__
	float guiScaling = 2.0f; // Added in version 2
#else
	float guiScaling = 1.0f; // Added in version 2
#endif
	nctl::String startupProjectName = nctl::String(ui::MaxStringLength); // Renamed in version 3
	bool autoPlayOnStart = true;

	nctl::String projectsPath = nctl::String(ui::MaxStringLength); // Renamed in version 3
	nctl::String texturesPath = nctl::String(ui::MaxStringLength);
	nctl::String scriptsPath = nctl::String(ui::MaxStringLength); // Added in version 3

	bool showTipsOnStart = true; // Added in version 4
	nctl::Array<nctl::String> pinnedDirectories = nctl::Array<nctl::String>(8); // Added in version 5
};

#endif
