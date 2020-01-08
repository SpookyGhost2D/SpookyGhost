#ifndef GUI_LABELS
#define GUI_LABELS

#ifdef WITH_FONTAWESOME
#include "IconsFontAwesome5.h"
#endif

#define TEXT_MENU_FILE_NEW "New"
#define TEXT_MENU_FILE_OPEN "Open"
#define TEXT_MENU_FILE_SAVE "Save"
#define TEXT_MENU_FILE_QUIT "Quit"

#define TEXT_HEADER_CANVAS "Canvas"
#define TEXT_HEADER_SPRITE "Sprite"
#define TEXT_HEADER_ANIMATIONS "Animations"
#define TEXT_HEADER_RENDER "Render"

#define TEXT_LOAD "Load"
#define TEXT_RESET "Reset"
#define TEXT_CLEAR "Clear"
#define TEXT_APPLY "Apply"

#define TEXT_ADD "Add"
#define TEXT_REMOVE "Remove"
#define TEXT_REMOVEALL "Remove All"

#define TEXT_STOP "Stop"
#define TEXT_PAUSE "Pause"
#define TEXT_PLAY "Play"

#ifndef WITH_FONTAWESOME
namespace Labels {
	static const char *INFO_MARKER = "(Info) ";
	static const char *ERROR_MARKER = "(Error) ";

	static const char *LoadingError = TEXT_LOADINGERROR;
	static const char *FileExists = TEXT_FILEEXISTS;
	static const char *Ok = TEXT_OK;
	static const char *Cancel = TEXT_CANCEL;

	static const char *New = TEXT_MENU_FILE_NEW;
	static const char *Open = TEXT_MENU_FILE_OPEN;
	static const char *Save = TEXT_MENU_FILE_SAVE;
	static const char *Quit = TEXT_MENU_FILE_QUIT;

	static const char *Canvas = TEXT_HEADER_CANVAS;
	static const char *Sprite = TEXT_HEADER_SPRITE;
	static const char *Animations = TEXT_HEADER_ANIMATIONS;
	static const char *Render = TEXT_HEADER_RENDER;

	static const char *Load = TEXT_LOAD;
	static const char *Reset = TEXT_RESET;
	static const char *Clear = TEXT_CLEAR;
	static const char *Apply = TEXT_APPLY;

	static const char *Add = TEXT_ADD;
	static const char *Remove = TEXT_REMOVE;
	static const char *RemoveAll = TEXT_REMOVEALL;

	static const char *Stop = TEXT_STOP;
	static const char *Pause = TEXT_PAUSE;
	static const char *Play = TEXT_PLAY;
}
#else
#define FA5_SPACING " "

namespace Labels {
	static const char *INFO_MARKER = ICON_FA_INFO FA5_SPACING;
	static const char *ERROR_MARKER = ICON_FA_EXCLAMATION_TRIANGLE FA5_SPACING;

	static const char *New = ICON_FA_FILE FA5_SPACING TEXT_MENU_FILE_NEW;
	static const char *Open = ICON_FA_FOLDER_OPEN FA5_SPACING TEXT_MENU_FILE_OPEN;
	static const char *Save = ICON_FA_SAVE FA5_SPACING TEXT_MENU_FILE_SAVE;
	static const char *Quit = ICON_FA_POWER_OFF FA5_SPACING TEXT_MENU_FILE_QUIT;

	static const char *Canvas = ICON_FA_PAINT_ROLLER FA5_SPACING TEXT_HEADER_CANVAS;
	static const char *Sprite = ICON_FA_GHOST FA5_SPACING TEXT_HEADER_SPRITE;
	static const char *Animations = ICON_FA_SLIDERS_H FA5_SPACING TEXT_HEADER_ANIMATIONS;
	static const char *Render = ICON_FA_IMAGE FA5_SPACING TEXT_HEADER_RENDER;

	static const char *Load = ICON_FA_FILE_UPLOAD FA5_SPACING TEXT_LOAD;
	static const char *Reset = ICON_FA_BACKSPACE FA5_SPACING TEXT_RESET;
	static const char *Clear = ICON_FA_BACKSPACE FA5_SPACING TEXT_CLEAR;
	static const char *Apply = ICON_FA_CHECK_CIRCLE FA5_SPACING TEXT_APPLY;

	static const char *Add = ICON_FA_PLUS FA5_SPACING TEXT_ADD;
	static const char *Remove = ICON_FA_MINUS FA5_SPACING TEXT_REMOVE;
	static const char *RemoveAll = ICON_FA_TRASH FA5_SPACING TEXT_REMOVEALL;

	static const char *Stop = ICON_FA_STOP FA5_SPACING TEXT_STOP;
	static const char *Pause = ICON_FA_PAUSE FA5_SPACING TEXT_PAUSE;
	static const char *Play = ICON_FA_PLAY FA5_SPACING TEXT_PLAY;
}
#endif

#endif
