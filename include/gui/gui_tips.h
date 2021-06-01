#ifndef GUI_TIPS
#define GUI_TIPS

#ifdef WITH_FONTAWESOME
	#include "IconsFontAwesome5.h"

	#define DOCUMENTATION_ITEM " (" ICON_FA_QUESTION_CIRCLE ") "
	#define CANVAS_WINDOW " (" ICON_FA_PAINT_ROLLER ") "
	#define SPRITE_WINDOW " (" ICON_FA_GHOST ") "
	#define TEXRECT_WINDOW " (" ICON_FA_CROP_ALT ") "
	#define TEXTURE_ICON " (" ICON_FA_IMAGE ") "
	#define CHECKMARK_ICON " (" ICON_FA_CHECK_CIRCLE ") "
	#define CROSS_ICON " (" ICON_FA_TIMES_CIRCLE ") "
	#define SPRITE_ICON " (" ICON_FA_GHOST ") "
	#define LOCK_ICON " (" ICON_FA_LOCK ") "
#else
	#define DOCUMENTATION_ITEM " "
	#define TEXRECT_WINDOW " "
	#define SPRITE_WINDOW " "
	#define SPRITE_WINDOW " "
	#define TEXTURE_ICON " [T] "
	#define CHECKMARK_ICON " [v] "
	#define CROSS_ICON " [x] "
	#define SPRITE_ICON "[S] "
	#define LOCK_ICON " [L] "
#endif

namespace Tips {

const unsigned int Count = 20;

// A string entry should always end with a comma or it would be concatenated to the next one
const char *Strings[Count] = {

	"Pressing [F1] will open the browser and load the documentation.\nYou can do the same by accessing the [Documentation] item" DOCUMENTATION_ITEM "in the [Help] menu.",
	"You can drag windows around and anchor them into any corner, change the split space between them or tab them one next to another.",
	"If you press [Ctrl] while clicking on a slider or a drag box you can input the value with the keyboard.",
	"You can modify the speed of change of drag boxes by pressing [Shift] (faster) or [Alt] (slower)",
	"You can drag and drop colors around, for example, the background color of the Canvas window" CANVAS_WINDOW "onto the sprite color in the Sprite window" SPRITE_WINDOW ".",
	"Pressing [F5] will save your work using a name based on date and time, so you can quick save multiple versions without ever overwriting any. Pressing [F9] will load the latest one.",
	"You can delete the selected texture, sprite, script, or animation with the [Delete] key when the mouse is over the relative window",
	"You can reorder sprites or animations with drag and drop.",
	"The texture icon" TEXTURE_ICON "at the end of a sprite entry indicates that the sprite is using the currently selected texture.",
	"The checkmark" CHECKMARK_ICON "or cross" CROSS_ICON "icon at the end of a script entry indicates that the script can be run or not.\nHovering with the mouse on a script that cannot run will show additional information about the error.",
	"You can reload a script by pressing [CTRL + R].",
	"Pressing [Space] when hovering on the Canvas window" CANVAS_WINDOW "will toggle the animation state between playing and paused.",
	"The sprite icon" SPRITE_ICON "at the end of an animation entry indicates that the animation is assigned to the currently selected sprite.\nWhen you select a different animation its assigned sprite will be automatically selected.",
	"You can use the TexRect window" TEXRECT_WINDOW "to help you set up the texture rectangle of a sprite using the mouse.",
	"The lock icon" LOCK_ICON "at the end of an animation entry indicates that the animation curve is locked.\nThe property, grid, or script will affect the sprite even if the animation is stopped or paused.",
	"You can change the canvas zoom level by hovering on the Canvas window" CANVAS_WINDOW ", pressing the [Ctrl] key and scrolling the mouse wheel.",
	"Pressing [Alt] while clicking on the Canvas window" CANVAS_WINDOW " will drag the selected sprite around.",
	"Pressing the arrow keys while the cursor is on the Canvas window" CANVAS_WINDOW " will move the selected sprite in one-pixel increments for precise positioning.",
	"Right-clicking on a texture, a sprite, a script, or an animation will show a contextual menu with a series of actions that you can perform.",
	"Animation groups have an option to override the sprite used by every contained animations. It comes in handy when you clone a group to apply the same animations to a different sprite.",
};

}

#endif
