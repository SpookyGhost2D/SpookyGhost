#include <ncine/imgui.h>
#include <ncine/InputEvents.h>
#include <ncine/Application.h>
#include "singletons.h"
#include "gui/gui_labels.h"
#include "gui/gui_common.h"
#include "gui/RenderGuiWindow.h"
#include "gui/UserInterface.h"
#include "gui/FileDialog.h"
#include "Canvas.h"
#include "AnimationManager.h"

namespace {

const char *ResizeStrings[7] = { "1/8X", "1/4X", "1/2X", "1X", "2X", "4X", "8X" };

}

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

RenderGuiWindow::RenderGuiWindow(UserInterface &ui)
    : ui_(ui)
{
#ifdef __ANDROID__
	filename = "animation";
#endif
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

float RenderGuiWindow::resizeAmount(ResizeLevel rl)
{
	switch (rl)
	{
		case ResizeLevel::X1_8:
			return 0.125f;
		case ResizeLevel::X1_4:
			return 0.25f;
		case ResizeLevel::X1_2:
			return 0.5f;
		case ResizeLevel::X1:
			return 1.0f;
		case ResizeLevel::X2:
			return 2.0f;
		case ResizeLevel::X4:
			return 4.0f;
		case ResizeLevel::X8:
			return 8.0f;
	}
	return 1.0f;
}

float RenderGuiWindow::resizeAmount() const
{
	return resizeAmount(resizeLevel);
}

void RenderGuiWindow::setResize(float resizeAmount)
{
	if (resizeAmount <= 0.125f)
		resizeLevel = ResizeLevel::X1_8;
	else if (resizeAmount <= 0.25f)
		resizeLevel = ResizeLevel::X1_4;
	else if (resizeAmount <= 0.5f)
		resizeLevel = ResizeLevel::X1_2;
	else if (resizeAmount <= 1.0f)
		resizeLevel = ResizeLevel::X1;
	else if (resizeAmount <= 2.0f)
		resizeLevel = ResizeLevel::X2;
	else if (resizeAmount <= 4.0f)
		resizeLevel = ResizeLevel::X4;
	else
		resizeLevel = ResizeLevel::X8;
}

void RenderGuiWindow::create()
{
	ImGui::Begin(Labels::Render);

	if (ImGui::IsWindowHovered() && ui::dropEvent != nullptr)
	{
		if (nc::fs::isDirectory(ui::dropEvent->paths[0]))
			directory = ui::dropEvent->paths[0];
		else
			directory = nc::fs::dirName(ui::dropEvent->paths[0]);
		ui::dropEvent = nullptr;
	}

	if (directory.isEmpty())
	{
#ifdef __ANDROID__
		directory.assign(ui::androidSaveDir);
#else
		directory.assign(nc::fs::currentDir());
#endif
	}
	if (shouldSaveFrames_ == false && shouldSaveSpritesheet_ == false)
	{
		ui::auxString.format("Save to: %s%s", Labels::FileDialog_SelectDirIcon, directory.data());
		if (ImGui::Button(ui::auxString.data()))
		{
			FileDialog::config.directory = directory;
			FileDialog::config.windowIcon = Labels::FileDialog_SelectDirIcon;
			FileDialog::config.windowTitle = "Render save directory";
			FileDialog::config.okButton = Labels::Ok;
			FileDialog::config.selectionType = FileDialog::SelectionType::DIRECTORY;
			FileDialog::config.extensions = nullptr;
			FileDialog::config.action = FileDialog::Action::RENDER_DIR;
			FileDialog::config.windowOpen = true;
		}
	}
	else
		ImGui::Text("%s", directory.data());

	int inputTextFlags = ImGuiInputTextFlags_CallbackResize;
	if (shouldSaveFrames_ || shouldSaveSpritesheet_)
		inputTextFlags |= ImGuiInputTextFlags_ReadOnly;
	ImGui::InputText("Filename prefix", filename.data(), ui::MaxStringLength,
	                 inputTextFlags, ui::inputTextCallback, &filename);

	int currentResizeCombo = static_cast<int>(resizeLevel);
	ui::comboString.clear();
	for (unsigned int i = 0; i < IM_ARRAYSIZE(ResizeStrings); i++)
	{
		const float resize = resizeAmount(ResizeLevel(i));
		ui::comboString.formatAppend("%s (%d x %d)", ResizeStrings[i], static_cast<int>(theCanvas->texWidth() * resize), static_cast<int>(theCanvas->texHeight() * resize));
		ui::comboString.setLength(ui::comboString.length() + 1);
	}
	ui::comboString.setLength(ui::comboString.length() + 1);
	// Append a second '\0' to signal the end of the combo item list
	ui::comboString[ui::comboString.length() - 1] = '\0';

	ImGui::Combo("Resize Level", &currentResizeCombo, ui::comboString.data());
	resizeLevel = static_cast<RenderGuiWindow::ResizeLevel>(currentResizeCombo);
	saveAnimStatus_.canvasResize = resizeAmount();

	const unsigned int MaxSliderSeconds = 10;
	ImGui::InputInt("FPS", &saveAnimStatus_.fps);
	ImGui::SliderInt("Num Frames", &saveAnimStatus_.numFrames, 1, MaxSliderSeconds * saveAnimStatus_.fps); // Hard-coded limit
	float duration = saveAnimStatus_.numFrames * saveAnimStatus_.inverseFps();
	ImGui::SliderFloat("Duration", &duration, 0.0f, MaxSliderSeconds, "%.3fs"); // Hard-coded limit

	const nc::Vector2i uncappedFrameSize(theCanvas->texWidth() * saveAnimStatus_.canvasResize, theCanvas->texHeight() * saveAnimStatus_.canvasResize);
	// Immediately-invoked function expression for const initialization
	const nc::Vector2i frameSize = [&] {
		nc::Vector2i size = uncappedFrameSize;
		size.x = (size.x > theCanvas->maxTextureSize()) ? theCanvas->maxTextureSize() : size.x;
		size.y = (size.y > theCanvas->maxTextureSize()) ? theCanvas->maxTextureSize() : size.y;
		return size;
	}();

	// Immediately-invoked function expression for const initialization
	const nc::Vector2i rectSides = [&] {
		const int rectSideX = static_cast<int>(ceilf(sqrtf(saveAnimStatus_.numFrames)));
		const int rectSideY = (rectSideX * (rectSideX - 1) >= saveAnimStatus_.numFrames) ? rectSideX - 1 : rectSideX;
		return nc::Vector2i(rectSideX, rectSideY);
	}();

	int currentLayoutCombo = static_cast<int>(layout);
	ui::comboString.clear();
	ui::comboString.formatAppend("HRectangle (%d x %d)", rectSides.x, rectSides.y);
	ui::comboString.setLength(ui::comboString.length() + 1);
	ui::comboString.formatAppend("VRectangle (%d x %d)", rectSides.y, rectSides.x);
	ui::comboString.setLength(ui::comboString.length() + 1);
	ui::comboString.formatAppend("HStrip (%d x %d)", saveAnimStatus_.numFrames, 1);
	ui::comboString.setLength(ui::comboString.length() + 1);
	ui::comboString.formatAppend("VStrip (%d x %d)", 1, saveAnimStatus_.numFrames);
	ui::comboString.setLength(ui::comboString.length() + 1);
	ui::comboString.append("Custom");
	ui::comboString.setLength(ui::comboString.length() + 1);

	ui::comboString.setLength(ui::comboString.length() + 1);
	// Append a second '\0' to signal the end of the combo item list
	ui::comboString[ui::comboString.length() - 1] = '\0';

	ImGui::Combo("Layout", &currentLayoutCombo, ui::comboString.data());
	layout = static_cast<RenderGuiWindow::SpritesheetLayout>(currentLayoutCombo);

	static nc::Vector2i customSides(rectSides);
	if (layout == SpritesheetLayout::CUSTOM)
	{
		static int newNumFrames = saveAnimStatus_.numFrames;
		nc::Vector2i newCustomSides(customSides);
		ImGui::InputInt2("H/V Sides", newCustomSides.data(), ImGuiInputTextFlags_EnterReturnsTrue);
		if (newCustomSides.x != customSides.x)
		{
			newCustomSides.y = static_cast<int>(ceilf(saveAnimStatus_.numFrames / static_cast<float>(newCustomSides.x)));
			// Avoid empty lines
			if (newCustomSides.x * newCustomSides.y > saveAnimStatus_.numFrames + newCustomSides.y)
				newCustomSides.x = static_cast<int>(ceilf(saveAnimStatus_.numFrames / static_cast<float>(newCustomSides.y)));
		}
		else if (newCustomSides.y != customSides.y)
		{
			newCustomSides.x = static_cast<int>(ceilf(saveAnimStatus_.numFrames / static_cast<float>(newCustomSides.y)));
			// Avoid empty lines
			if (newCustomSides.x * newCustomSides.y > saveAnimStatus_.numFrames + newCustomSides.x)
				newCustomSides.y = static_cast<int>(ceilf(saveAnimStatus_.numFrames / static_cast<float>(newCustomSides.x)));
		}

		if (newNumFrames != saveAnimStatus_.numFrames &&
		    (newCustomSides.x * newCustomSides.y < saveAnimStatus_.numFrames ||
		     newCustomSides.x * newCustomSides.y > saveAnimStatus_.numFrames + newCustomSides.x))
		{
			newCustomSides = rectSides;
			newNumFrames = saveAnimStatus_.numFrames;
		}

		customSides = newCustomSides;
	}

	nc::Vector2i uncappedSides(rectSides);
	switch (layout)
	{
		case SpritesheetLayout::HRECTANGLE:
			uncappedSides = rectSides;
			break;
		case SpritesheetLayout::VRECTANGLE:
			uncappedSides.set(rectSides.y, rectSides.x);
			break;
		case SpritesheetLayout::HSTRIP:
			uncappedSides.set(saveAnimStatus_.numFrames, 1);
			break;
		case SpritesheetLayout::VSTRIP:
			uncappedSides.set(1, saveAnimStatus_.numFrames);
			break;
		case SpritesheetLayout::CUSTOM:
			uncappedSides = customSides;
			break;
	}

	const nc::Vector2i uncappedSpritesheetSize(uncappedSides.x * frameSize.x, uncappedSides.y * frameSize.y);
	// Immediately-invoked function expression for const initialization
	const nc::Vector2i spritesheetSize = [&] {
		nc::Vector2i size = uncappedSpritesheetSize;
		size.x = (size.x > theCanvas->maxTextureSize()) ? theCanvas->maxTextureSize() : size.x;
		size.y = (size.y > theCanvas->maxTextureSize()) ? theCanvas->maxTextureSize() : size.y;
		return size;
	}();

	// The resize combo already shows uncapped frame size information
	if (frameSize.x != uncappedFrameSize.x || frameSize.y != uncappedFrameSize.y)
	{
		ui::auxString.format("%d x %d", frameSize.x, frameSize.y);
		if (uncappedFrameSize.x > frameSize.x || uncappedFrameSize.y > frameSize.y)
			ui::auxString.formatAppend(" (capped from %d x %d)", uncappedFrameSize.x, uncappedFrameSize.y);
		ImGui::Text("Frame size: %s", ui::auxString.data());
	}

	ui::auxString.format("%d x %d", spritesheetSize.x, spritesheetSize.y);
	if (uncappedSpritesheetSize.x > spritesheetSize.x || uncappedSpritesheetSize.y > spritesheetSize.y)
		ui::auxString.formatAppend(" (capped from %d x %d)", uncappedSpritesheetSize.x, uncappedSpritesheetSize.y);
	ImGui::Text("Spritesheet size: %s", ui::auxString.data());

	// The layout combo already shows uncapped sides information
	const nc::Vector2i sheetSides(spritesheetSize / frameSize);
	if (sheetSides.x != uncappedSides.x || sheetSides.y != uncappedSides.y)
	{
		ui::auxString.format("%d x %d (%d)", sheetSides.x, sheetSides.y, sheetSides.x * sheetSides.y);
		if (uncappedSides.x > sheetSides.x || uncappedSides.y > sheetSides.y)
			ui::auxString.formatAppend(" (capped from %d x %d)", uncappedSides.x, uncappedSides.y);
		ImGui::Text("Spritesheet layout: %s", ui::auxString.data());
	}

	saveAnimStatus_.numFrames = static_cast<int>(duration * saveAnimStatus_.fps);
	if (saveAnimStatus_.numFrames < 1)
		saveAnimStatus_.numFrames = 1;

	if (shouldSaveFrames_ || shouldSaveSpritesheet_)
	{
		const unsigned int numSavedFrames = saveAnimStatus_.numSavedFrames;
		const float fraction = numSavedFrames / static_cast<float>(saveAnimStatus_.numFrames);
		ui::auxString.format("Frame: %d/%d", numSavedFrames, saveAnimStatus_.numFrames);
		ImGui::ProgressBar(fraction, ImVec2(0.0f, 0.0f), ui::auxString.data());
		ImGui::SameLine();
		if (ImGui::Button(Labels::Cancel))
			cancelRender();
	}
	else
	{
		if (ImGui::Button(Labels::SaveFrames))
		{
			if (filename.isEmpty())
				ui_.pushStatusErrorMessage("Set a filename prefix before saving an animation");
			else
			{
				saveAnimStatus_.filename.format("%s_%03d.png", nc::fs::joinPath(directory, filename).data(), saveAnimStatus_.numSavedFrames);
				shouldSaveFrames_ = true;
				theResizedCanvas->resizeTexture(frameSize);
				// Disabling V-Sync for faster render times
				nc::theApplication().gfxDevice().setSwapInterval(0);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button(Labels::SaveSpritesheet))
		{
			if (filename.isEmpty())
				ui_.pushStatusErrorMessage("Set a filename prefix before saving an animation");
			else
			{
				saveAnimStatus_.filename.format("%s.png", nc::fs::joinPath(directory, filename).data(), saveAnimStatus_.numSavedFrames);
				shouldSaveSpritesheet_ = true;
				saveAnimStatus_.sheetDestPos.set(0, 0);
				theResizedCanvas->resizeTexture(theCanvas->size() * saveAnimStatus_.canvasResize);
				theSpritesheet->resizeTexture(spritesheetSize);
				// Disabling V-Sync for faster render times
				nc::theApplication().gfxDevice().setSwapInterval(0);
			}
		}
	}
	ImGui::End();
}

void RenderGuiWindow::signalFrameSaved()
{
	ASSERT(shouldSaveFrames_ || shouldSaveSpritesheet_);

	saveAnimStatus_.numSavedFrames++;
	if (shouldSaveFrames_)
		saveAnimStatus_.filename.format("%s_%03d.png", nc::fs::joinPath(directory, filename).data(), saveAnimStatus_.numSavedFrames);
	else if (shouldSaveSpritesheet_)
	{
		Canvas &sourceCanvas = (saveAnimStatus_.canvasResize != 1.0f) ? *theResizedCanvas : *theCanvas;
		saveAnimStatus_.sheetDestPos.x += sourceCanvas.texWidth();
		if (saveAnimStatus_.sheetDestPos.x + sourceCanvas.texWidth() > theSpritesheet->texWidth())
		{
			saveAnimStatus_.sheetDestPos.x = 0;
			saveAnimStatus_.sheetDestPos.y += sourceCanvas.texHeight();
		}
	}
	if (saveAnimStatus_.numSavedFrames == saveAnimStatus_.numFrames)
	{
		shouldSaveFrames_ = false;
		shouldSaveSpritesheet_ = false;
		saveAnimStatus_.numSavedFrames = 0;
		ui_.pushStatusInfoMessage("Animation saved");

		// Re-enabling V-Sync if it was enabled in the configuration
		if (theCfg.withVSync)
			nc::theApplication().gfxDevice().setSwapInterval(1);
	}
}

void RenderGuiWindow::cancelRender()
{
	if (shouldSaveFrames_ || shouldSaveSpritesheet_)
	{
		if (shouldSaveFrames_)
			ui::auxString.format("Render cancelled, saved %d out of %d frames", saveAnimStatus_.numSavedFrames, saveAnimStatus_.numFrames);
		else if (shouldSaveSpritesheet_)
			ui::auxString = "Render cancelled, the spritesheet has not been saved";
		ui_.pushStatusInfoMessage(ui::auxString.data());
		saveAnimStatus_.numSavedFrames = 0;
		shouldSaveFrames_ = false;
		shouldSaveSpritesheet_ = false;

		// Re-enabling V-Sync if it was enabled in the configuration
		if (theCfg.withVSync)
			nc::theApplication().gfxDevice().setSwapInterval(1);
	}
}
