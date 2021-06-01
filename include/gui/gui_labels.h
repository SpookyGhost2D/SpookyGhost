#ifndef GUI_LABELS
#define GUI_LABELS

#ifdef WITH_FONTAWESOME
	#include "IconsFontAwesome5.h"
#endif

#define TEXT_LOADINGERROR "Loading error!"
#define TEXT_FILEEXISTS "File exists!"
#define TEXT_OK "Ok"
#define TEXT_CANCEL "Cancel"
#define TEXT_CLOSE "Close"

#define TEXT_MENU_FILE_NEW "New"
#define TEXT_MENU_FILE_OPEN "Open"
#define TEXT_MENU_FILE_OPENBUNDLED "Open Bundled"
#define TEXT_MENU_FILE_SAVE "Save"
#define TEXT_MENU_FILE_SAVEAS "Save as..."
#define TEXT_MENU_FILE_QUICKOPEN "Quick Open"
#define TEXT_MENU_FILE_QUICKSAVE "Quick Save"
#define TEXT_MENU_FILE_CONFIGURATION "Configuration"
#define TEXT_MENU_FILE_QUIT "Quit"
#define TEXT_MENU_DOCUMENTATION "Documentation"
#define TEXT_MENU_TIPS "Tips"
#define TEXT_MENU_ABOUT "About"

#define TEXT_HEADER_CANVAS "Canvas"
#define TEXT_HEADER_TEXRECT "TexRect"
#define TEXT_HEADER_TEXTURES "Textures"
#define TEXT_HEADER_SPRITES "Sprites"
#define TEXT_HEADER_SCRIPTS "Scripts"
#define TEXT_HEADER_ANIMATIONS "Animations"
#define TEXT_HEADER_RENDER "Render"

#define TEXT_HEADER_SPRITE "Sprite"
#define TEXT_HEADER_ANIMATION "Animation"

#define TEXT_COMBO_BUNDLED_TEXTURES "Bundled Textures"
#define TEXT_COMBO_BUNDLED_SCRIPTS "Bundled Scripts"

#define TEXT_LOAD "Load"
#define TEXT_RELOAD "Reload"
#define TEXT_RESET "Reset"
#define TEXT_CLEAR "Clear"
#define TEXT_APPLY "Apply"
#define TEXT_CURRENT "Current"
#define TEXT_LOCKED "Locked"
#define TEXT_SELECT_PARENT "Select Parent"

#define TEXT_ADD "Add"
#define TEXT_REMOVE "Remove"
#define TEXT_CLONE "Clone"
#define TEXT_MOVE_UP "Move Up"
#define TEXT_MOVE_DOWN "Move Down"

#define TEXT_STOP "Stop"
#define TEXT_PAUSE "Pause"
#define TEXT_PLAY "Play"

#define TEXT_SAVE_FRAMES "Save Frames"
#define TEXT_SAVE_SPRITESHEET "Save Spritesheet"

#ifndef WITH_FONTAWESOME
namespace Labels {

static const char *INFO_MARKER = "(Info) ";
static const char *ERROR_MARKER = "(Error) ";

static const char *LoadingError = TEXT_LOADINGERROR;
static const char *FileExists = TEXT_FILEEXISTS;
static const char *Ok = TEXT_OK;
static const char *Cancel = TEXT_CANCEL;
static const char *Close = TEXT_CLOSE;

static const char *New = TEXT_MENU_FILE_NEW;
static const char *Open = TEXT_MENU_FILE_OPEN;
static const char *OpenBundled = TEXT_MENU_FILE_OPENBUNDLED;
static const char *Save = TEXT_MENU_FILE_SAVE;
static const char *SaveAs = TEXT_MENU_FILE_SAVEAS;
static const char *QuickOpen = TEXT_MENU_FILE_QUICKOPEN;
static const char *QuickSave = TEXT_MENU_FILE_QUICKSAVE;
static const char *Configuration = TEXT_MENU_FILE_CONFIGURATION;
static const char *Quit = TEXT_MENU_FILE_QUIT;
static const char *Documentation = TEXT_MENU_DOCUMENTATION;
static const char *Tips = TEXT_MENU_TIPS;
static const char *About = TEXT_MENU_ABOUT;

static const char *Canvas = TEXT_HEADER_CANVAS;
static const char *TexRect = TEXT_HEADER_TEXRECT;
static const char *Textures = TEXT_HEADER_TEXTURES;
static const char *Sprites = TEXT_HEADER_SPRITES;
static const char *Scripts = TEXT_HEADER_SCRIPTS;
static const char *Animations = TEXT_HEADER_ANIMATIONS;
static const char *Render = TEXT_HEADER_RENDER;

static const char *Sprite = TEXT_HEADER_SPRITE;
static const char *Animation = TEXT_HEADER_ANIMATION;

static const char *Load = TEXT_LOAD;
static const char *Reload = TEXT_RELOAD;
static const char *Reset = TEXT_RESET;
static const char *Clear = TEXT_CLEAR;
static const char *Apply = TEXT_APPLY;
static const char *Current = TEXT_CURRENT;
static const char *Locked = TEXT_LOCKED;
static const char *SelectParent = TEXT_SELECT_PARENT;

static const char *Add = TEXT_ADD;
static const char *Remove = TEXT_REMOVE;
static const char *Clone = TEXT_CLONE;
static const char *MoveUp = TEXT_MOVE_UP;
static const char *MoveDown = TEXT_MOVE_DOWN;

static const char *Stop = TEXT_STOP;
static const char *Pause = TEXT_PAUSE;
static const char *Play = TEXT_PLAY;

static const char *SaveFrames = TEXT_SAVE_FRAMES;
static const char *SaveSpritesheet = TEXT_SAVE_SPRITESHEET;

static const char *BundledTexture = TEXT_COMBO_BUNDLED_TEXTURES;
static const char *BundledScripts = TEXT_COMBO_BUNDLED_SCRIPTS;

static const char *VisibleIcon = "V";
static const char *InvisibleIcon = "I";
static const char *EnabledAnimIcon = "[>]";
static const char *DisabledAnimIcon = "[ ]";
static const char *PlusIcon = "+";
static const char *MinusIcon = "-";
static const char *PreviousIcon = "<";
static const char *NextIcon = ">";
static const char *CheckIcon = "[v]";
static const char *TimesIcon = "[x]";
static const char *LightbulbIcon = "[*]";
static const char *SelectedIcon = "[*]";
static const char *SelectedTextureIcon = "[T]";
static const char *SelectedSpriteIcon = "[S]";
static const char *LockedAnimIcon = "[L]";
static const char *StopIcon = "[Stop]";
static const char *PauseIcon = "[Pause]";
static const char *PlayIcon = "[Play]";

static const char *FileDialog_OpenIcon = "";
static const char *FileDialog_SaveIcon = "";
static const char *FileDialog_SelectDirIcon = "";
static const char *FileDialog_FolderIcon = "[Dir ]";
static const char *FileDialog_FileIcon = "[File]";
static const char *FileDialog_Sorting = "Sorting";
static const char *FileDialog_Name_Asc = "Name Asc";
static const char *FileDialog_Name_Desc = "Name Desc";
static const char *FileDialog_Size_Asc = "Size Asc";
static const char *FileDialog_Size_Desc = "Size Desc";
static const char *FileDialog_Date_Asc = "Date Asc";
static const char *FileDialog_Date_Desc = "Date Desc";

}
#else
	#define FA5_SPACING " "

namespace Labels {

static const char *INFO_MARKER = ICON_FA_INFO FA5_SPACING;
static const char *ERROR_MARKER = ICON_FA_EXCLAMATION_TRIANGLE FA5_SPACING;

static const char *LoadingError = ICON_FA_EXCLAMATION_TRIANGLE FA5_SPACING TEXT_LOADINGERROR;
static const char *FileExists = ICON_FA_EXCLAMATION_TRIANGLE FA5_SPACING TEXT_FILEEXISTS;
static const char *Ok = ICON_FA_CHECK_CIRCLE FA5_SPACING TEXT_OK;
static const char *Cancel = ICON_FA_TIMES_CIRCLE FA5_SPACING TEXT_CANCEL;
static const char *Close = ICON_FA_TIMES_CIRCLE FA5_SPACING TEXT_CLOSE;

static const char *New = ICON_FA_FILE FA5_SPACING TEXT_MENU_FILE_NEW;
static const char *Open = ICON_FA_FOLDER_OPEN FA5_SPACING TEXT_MENU_FILE_OPEN;
static const char *OpenBundled = ICON_FA_FOLDER_OPEN FA5_SPACING TEXT_MENU_FILE_OPENBUNDLED;
static const char *Save = ICON_FA_SAVE FA5_SPACING TEXT_MENU_FILE_SAVE;
static const char *SaveAs = ICON_FA_SAVE FA5_SPACING TEXT_MENU_FILE_SAVEAS;
static const char *QuickOpen = ICON_FA_FOLDER_OPEN FA5_SPACING TEXT_MENU_FILE_QUICKOPEN;
static const char *QuickSave = ICON_FA_SAVE FA5_SPACING TEXT_MENU_FILE_QUICKSAVE;
static const char *Configuration = ICON_FA_TOOLS FA5_SPACING TEXT_MENU_FILE_CONFIGURATION;
static const char *Quit = ICON_FA_POWER_OFF FA5_SPACING TEXT_MENU_FILE_QUIT;
static const char *Documentation = ICON_FA_QUESTION_CIRCLE FA5_SPACING TEXT_MENU_DOCUMENTATION;
static const char *Tips = ICON_FA_LIGHTBULB FA5_SPACING TEXT_MENU_TIPS;
static const char *About = ICON_FA_INFO_CIRCLE FA5_SPACING TEXT_MENU_ABOUT;

static const char *Canvas = ICON_FA_PAINT_ROLLER FA5_SPACING TEXT_HEADER_CANVAS;
static const char *TexRect = ICON_FA_CROP_ALT FA5_SPACING TEXT_HEADER_TEXRECT;
static const char *Textures = ICON_FA_IMAGE FA5_SPACING TEXT_HEADER_TEXTURES;
static const char *Sprites = ICON_FA_GHOST FA5_SPACING TEXT_HEADER_SPRITES;
static const char *Scripts = ICON_FA_SCROLL FA5_SPACING TEXT_HEADER_SCRIPTS;
static const char *Animations = ICON_FA_SLIDERS_H FA5_SPACING TEXT_HEADER_ANIMATIONS;
static const char *Render = ICON_FA_IMAGE FA5_SPACING TEXT_HEADER_RENDER;

static const char *Sprite = ICON_FA_GHOST FA5_SPACING TEXT_HEADER_SPRITE;
static const char *Animation = ICON_FA_SLIDERS_H FA5_SPACING TEXT_HEADER_ANIMATION;

static const char *Load = ICON_FA_FILE_UPLOAD FA5_SPACING TEXT_LOAD;
static const char *Reload = ICON_FA_REDO FA5_SPACING TEXT_RELOAD;
static const char *Reset = ICON_FA_BACKSPACE FA5_SPACING TEXT_RESET;
static const char *Clear = ICON_FA_BACKSPACE FA5_SPACING TEXT_CLEAR;
static const char *Apply = ICON_FA_CHECK_CIRCLE FA5_SPACING TEXT_APPLY;
static const char *Current = ICON_FA_SYNC FA5_SPACING TEXT_CURRENT;
static const char *Locked = ICON_FA_LOCK;
static const char *SelectParent = ICON_FA_OBJECT_GROUP FA5_SPACING TEXT_SELECT_PARENT;

static const char *Add = ICON_FA_PLUS FA5_SPACING TEXT_ADD;
static const char *Remove = ICON_FA_MINUS FA5_SPACING TEXT_REMOVE;
static const char *Clone = ICON_FA_CLONE FA5_SPACING TEXT_CLONE;
static const char *MoveUp = ICON_FA_ANGLE_UP FA5_SPACING TEXT_MOVE_UP;
static const char *MoveDown = ICON_FA_ANGLE_DOWN FA5_SPACING TEXT_MOVE_DOWN;

static const char *Stop = ICON_FA_STOP FA5_SPACING TEXT_STOP;
static const char *Pause = ICON_FA_PAUSE FA5_SPACING TEXT_PAUSE;
static const char *Play = ICON_FA_PLAY FA5_SPACING TEXT_PLAY;

static const char *SaveFrames = ICON_FA_SAVE FA5_SPACING TEXT_SAVE_FRAMES;
static const char *SaveSpritesheet = ICON_FA_SAVE FA5_SPACING TEXT_SAVE_SPRITESHEET;

static const char *BundledTextures = ICON_FA_FOLDER_OPEN FA5_SPACING TEXT_COMBO_BUNDLED_TEXTURES;
static const char *BundledScripts = ICON_FA_FOLDER_OPEN FA5_SPACING TEXT_COMBO_BUNDLED_SCRIPTS;

static const char *VisibleIcon = ICON_FA_EYE;
static const char *InvisibleIcon = ICON_FA_EYE_SLASH;
static const char *EnabledAnimIcon = ICON_FA_PLAY_CIRCLE;
static const char *DisabledAnimIcon = ICON_FA_STOP_CIRCLE;
static const char *PlusIcon = ICON_FA_PLUS;
static const char *MinusIcon = ICON_FA_MINUS;
static const char *PreviousIcon = ICON_FA_CARET_LEFT;
static const char *NextIcon = ICON_FA_CARET_RIGHT;
static const char *CheckIcon = ICON_FA_CHECK_CIRCLE;
static const char *TimesIcon = ICON_FA_TIMES_CIRCLE;
static const char *LightbulbIcon = ICON_FA_LIGHTBULB;
static const char *SelectedIcon = ICON_FA_CHECK;
static const char *SelectedTextureIcon = ICON_FA_IMAGE;
static const char *SelectedSpriteIcon = ICON_FA_GHOST;
static const char *LockedAnimIcon = ICON_FA_LOCK;
static const char *StopIcon = ICON_FA_STOP;
static const char *PauseIcon = ICON_FA_PAUSE;
static const char *PlayIcon = ICON_FA_PLAY;

static const char *FileDialog_OpenIcon = ICON_FA_FOLDER_OPEN FA5_SPACING;
static const char *FileDialog_SaveIcon = ICON_FA_SAVE FA5_SPACING;
static const char *FileDialog_SelectDirIcon = ICON_FA_FOLDER FA5_SPACING;
static const char *FileDialog_FolderIcon = ICON_FA_FOLDER;
static const char *FileDialog_FileIcon = ICON_FA_FILE;
static const char *FileDialog_Sorting = ICON_FA_SORT FA5_SPACING "Sorting";
static const char *FileDialog_Name_Asc = ICON_FA_SORT_ALPHA_DOWN FA5_SPACING "Name";
static const char *FileDialog_Name_Desc = ICON_FA_SORT_ALPHA_UP FA5_SPACING "Name";
static const char *FileDialog_Size_Asc = ICON_FA_SORT_AMOUNT_UP FA5_SPACING "Size";
static const char *FileDialog_Size_Desc = ICON_FA_SORT_AMOUNT_DOWN FA5_SPACING "Size";
static const char *FileDialog_Date_Asc = ICON_FA_SORT_NUMERIC_DOWN FA5_SPACING "Date";
static const char *FileDialog_Date_Desc = ICON_FA_SORT_NUMERIC_UP FA5_SPACING "Date";

}
#endif

#endif
