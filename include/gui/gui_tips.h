#ifndef GUI_TIPS
#define GUI_TIPS

#include "gui_labels.h"
#include <nctl/StaticArray.h>
#include <nctl/String.h>

namespace Tips {

/// The longest tip string will be wrapped to the following number of lines by the ImGui::Text widget
const unsigned int MaxNumberLines = 5;

const unsigned int Count = 29;
nctl::StaticArray<nctl::String, Count> Strings;

void initStrings()
{
	nctl::String temp(512);
	Strings.pushBack(temp.format("Pressing [F1] will open the browser and load the documentation.\nYou can do the same by accessing the \"%s\" item in the [Help] menu.", Labels::Documentation));
	Strings.pushBack("You can drag windows around and anchor them into any corner, change the split space between them or tab them one next to another.");
	Strings.pushBack("If you press [Ctrl] while clicking on a slider or a drag box you can input the value with the keyboard.");
	Strings.pushBack("You can modify the speed of change of drag boxes by pressing [Shift] (faster) or [Alt] (slower)");
	Strings.pushBack(temp.format("You can drag and drop colors around. For example, the background color of the \"%s\" window onto the sprite color in the \"%s\" window.", Labels::Canvas, Labels::Sprite));
	Strings.pushBack("Pressing [F5] will save your work using a name based on date and time, so you can quick save multiple versions without ever overwriting any. Pressing [F9] will load the latest one.");
	Strings.pushBack("You can delete the selected texture, sprite, script, or animation with the [Delete] key when the mouse is over the relative window.");
	Strings.pushBack("You can reorder sprites or animations with drag and drop.");
	Strings.pushBack(temp.format("The texture icon (%s) at the end of a sprite entry indicates that the sprite is using the currently selected texture.", Labels::SelectedTextureIcon));
	Strings.pushBack(temp.format("The checkmark (%s) or cross (%s) icon at the end of a script entry indicates that the script can be run or not.\nHovering with the mouse on a script that cannot run will show additional information about the error.", Labels::CheckIcon, Labels::TimesIcon));
	Strings.pushBack("You can reload a script by pressing [CTRL + R].");
	Strings.pushBack(temp.format("Pressing [Space] when hovering on the \"%s\" window will toggle the animation state between playing and paused.", Labels::Canvas));
	Strings.pushBack(temp.format("The sprite icon (%s) at the end of an animation entry indicates that the animation is assigned to the currently selected sprite.\nWhen you select a different animation its assigned sprite will be automatically selected.", Labels::SelectedSpriteIcon));
	Strings.pushBack(temp.format("You can use the \"%s\" window to help you set up the texture rectangle of a sprite using the mouse.", Labels::TexRect));
	Strings.pushBack(temp.format("The lock icon (%s) at the end of an animation entry indicates that the animation curve is locked.\nThe property, grid, or script will affect the sprite even if the animation is stopped or paused.", Labels::LockedAnimIcon));
	Strings.pushBack(temp.format("You can change the canvas zoom level by hovering on the \"%s\" window pressing the [Ctrl] key and scrolling the mouse wheel.", Labels::Canvas));
	Strings.pushBack(temp.format("Pressing [Alt] while clicking on the \"%s\" window will drag the selected sprite around.", Labels::Canvas));
	Strings.pushBack(temp.format("Pressing the arrow keys while the cursor is on the \"%s\" window will move the selected sprite in one-pixel increments for precise positioning.", Labels::Canvas));
	Strings.pushBack("Right-clicking on a texture, a sprite, a script, or an animation will show a contextual menu with a series of actions that you can perform.");
	Strings.pushBack("Animation groups have an option to override the sprite used by every contained animations. It comes in handy when you clone a group to apply the same animations to a different sprite.");
	Strings.pushBack(temp.format("Press [CTRL] while selecting a sprite or a sprite group (%s) in the \"%s\" window or an animation in the \"%s\" window to change its name.", Labels::GroupIcon, Labels::Sprite, Labels::Animations));
	Strings.pushBack(temp.format("Pressing the up or down arrow key when hovering on the \"%s\" or the \"%s\" windows will move sprites and animations up or down.", Labels::Sprites, Labels::Animations));
	Strings.pushBack(temp.format("Pressing the left or right arrow key when hovering on the \"%s\" window will show either the previous or the next tip.", Labels::Tips));
	Strings.pushBack(temp.format("In the \"%s\" window, press [ALT] to drag the texture rectangle around, and [SHIFT] to resize it from one corner.", Labels::TexRect));
	Strings.pushBack(temp.format("In a file dialog window, press the \"%s\" button to save the current directory to the list on the left.\nYou can then click it to navigate there with the browser.", Labels::FileDialog_Pin));
	Strings.pushBack(temp.format("Drag directories onto the list of pinned ones on the left side of a file dialog window to add them to the list."));
	Strings.pushBack(temp.format("Drag a file or a directory onto the browser on the right side of a file dialog window to navigate to that file or directory."));
	Strings.pushBack(temp.format("Drag files to the \"%s\" or \"%s\" windows to load them.", Labels::Textures, Labels::Scripts));
	Strings.pushBack(temp.format("Drag a file or a directory to the \"%s\" window to use it as a save directory for rendering.", Labels::Render));

	ASSERT(Strings.size() == Count);
}

}

#endif
