#include <ncine/imgui.h>
#include <ncine/imgui_internal.h>
#include <ncine/Application.h>
#include <ncine/IFile.h>

#include "gui_labels.h"
#include "UserInterface.h"
#include "Canvas.h"
#include "SpriteManager.h"
#include "AnimationManager.h"
#include "PropertyAnimation.h"
#include "ParallelAnimationGroup.h"
#include "SequentialAnimationGroup.h"
#include "GridAnimation.h"
#include "GridFunctionLibrary.h"
#include "Sprite.h"
#include "Texture.h"

#include "version.h"
#include <ncine/version.h>

namespace {

// clang-format off
const char *anchorPointItems[] = { "Center", "Bottom Left", "Top Left", "Bottom Right", "Top Right" };
enum AnchorPointsEnum { CENTER, BOTTOM_LEFT, TOP_LEFT, BOTTOM_RIGHT, TOP_RIGHT };
const char *blendingPresets[] = { "Disabled", "Alpha", "Pre-multiplied Alpha", "Additive", "Multiply" };

const char *easingCurveTypes[] = { "Linear", "Quadratic", "Cubic", "Quartic", "Quintic", "Sine", "Exponential", "Circular" };
const char *easingCurveDirections[] = { "Forward", "Backward" };
const char *easingCurveLoopModes[] = { "Disabled", "Rewind", "Ping Pong" };

const char *animationTypes[] = { "Parallel Group", "Sequential Group", "Property", "Grid" };
enum AnimationTypesEnum { PARALLEL_GROUP, SEQUENTIAL_GROUP, PROPERTY, GRID };

const char *propertyTypes[] = { "None", "Position X", "Position Y", "Rotation", "Scale X", "Scale Y", "AnchorPoint X", "AnchorPoint Y", "Opacity", "Red Channel", "Green Channel", "Blue Channel" };
enum PropertyTypesEnum { NONE, POSITION_X, POSITION_Y, ROTATION, SCALE_X, SCALE_Y, ANCHOR_X, ANCHOR_Y, OPACITY, COLOR_R, COLOR_G, COLOR_B };

const char *resizePresets[] = { "16x16", "32x32", "64x64", "128x128", "256x256", "512x512", "Custom" };
enum ResizePresetsEnum { SIZE16, SIZE32, SIZE64, SIZE128, SIZE256, SIZE512, CUSTOM };

enum CanvasZoomEnum { X1_8, X1_4, X1_2, X1, X2, X4, X8 };
// clang-format on

static bool showAboutWindow = false;
static bool showTexrectWindow = false;
static bool hoveringOnCanvas = false;

int inputTextCallback(ImGuiInputTextCallbackData *data)
{
	nctl::String *string = reinterpret_cast<nctl::String *>(data->UserData);
	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
	{
		// Resize string callback
		ASSERT(data->Buf == string->data());
		string->setLength(static_cast<unsigned int>(data->BufTextLen));
		data->Buf = string->data();
	}
	return 0;
}

const char *animStateToString(IAnimation::State state)
{
	switch (state)
	{
		case IAnimation::State::STOPPED: return "Stopped";
		case IAnimation::State::PAUSED: return "Paused";
		case IAnimation::State::PLAYING: return "Playing";
	}
	return "Unknown";
}

CanvasZoomEnum CanvasZoomToEnum(float zoom)
{
	if (zoom <= 0.125f)
		return CanvasZoomEnum::X1_8;
	else if (zoom <= 0.25f)
		return CanvasZoomEnum::X1_4;
	else if (zoom <= 0.5f)
		return CanvasZoomEnum::X1_2;
	else if (zoom <= 1.0f)
		return CanvasZoomEnum::X1;
	else if (zoom <= 2.0f)
		return CanvasZoomEnum::X2;
	else if (zoom <= 4.0f)
		return CanvasZoomEnum::X4;
	else
		return CanvasZoomEnum::X8;
}

float CanvasEnumToZoom(CanvasZoomEnum resizeEnum)
{
	switch (resizeEnum)
	{
		case CanvasZoomEnum::X1_8:
			return 0.125f;
		case CanvasZoomEnum::X1_4:
			return 0.25f;
		case CanvasZoomEnum::X1_2:
			return 0.5f;
		case CanvasZoomEnum::X1:
			return 1.0f;
		case CanvasZoomEnum::X2:
			return 2.0f;
		case CanvasZoomEnum::X4:
			return 4.0f;
		case CanvasZoomEnum::X8:
			return 8.0f;
	}
	return 1.0f;
}

void applyDarkStyle()
{
	ImGuiStyle &style = ImGui::GetStyle();
	ImVec4 *colors = style.Colors;

	/// 0 = FLAT APPEARENCE
	/// 1 = MORE "3D" LOOK
	const float is3D = 1.0f;

	// clang-format off
	colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled]           = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
	colors[ImGuiCol_ChildBg]                = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	colors[ImGuiCol_WindowBg]               = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	colors[ImGuiCol_PopupBg]                = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	colors[ImGuiCol_Border]                 = ImVec4(0.12f, 0.12f, 0.12f, 0.71f);
	colors[ImGuiCol_BorderShadow]           = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
	colors[ImGuiCol_FrameBg]                = ImVec4(0.42f, 0.42f, 0.42f, 0.54f);
	colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.42f, 0.42f, 0.42f, 0.40f);
	colors[ImGuiCol_FrameBgActive]          = ImVec4(0.56f, 0.56f, 0.56f, 0.67f);
	colors[ImGuiCol_TitleBg]                = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
	colors[ImGuiCol_TitleBgActive]          = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.17f, 0.17f, 0.17f, 0.90f);
	colors[ImGuiCol_MenuBarBg]              = ImVec4(0.335f, 0.335f, 0.335f, 1.000f);
	colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.24f, 0.24f, 0.24f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
	colors[ImGuiCol_CheckMark]              = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);
	colors[ImGuiCol_SliderGrab]             = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
	colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.64f, 0.64f, 0.64f, 1.00f);
	colors[ImGuiCol_Button]                 = ImVec4(0.54f, 0.54f, 0.54f, 0.35f);
	colors[ImGuiCol_ButtonHovered]          = ImVec4(0.52f, 0.52f, 0.52f, 0.59f);
	colors[ImGuiCol_ButtonActive]           = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
	colors[ImGuiCol_Header]                 = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
	colors[ImGuiCol_HeaderHovered]          = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
	colors[ImGuiCol_HeaderActive]           = ImVec4(0.76f, 0.76f, 0.76f, 0.77f);
	colors[ImGuiCol_Separator]              = ImVec4(0.000f, 0.000f, 0.000f, 0.137f);
	colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.700f, 0.671f, 0.600f, 0.290f);
	colors[ImGuiCol_SeparatorActive]        = ImVec4(0.702f, 0.671f, 0.600f, 0.674f);
	colors[ImGuiCol_ResizeGrip]             = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
	colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.73f, 0.73f, 0.73f, 0.35f);
	colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
	colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);

	style.PopupRounding = 3;

	style.WindowPadding = ImVec2(4, 4);
	style.FramePadding  = ImVec2(6, 4);
	style.ItemSpacing   = ImVec2(6, 2);

	style.ScrollbarSize = 18;

	style.WindowBorderSize = 1;
	style.ChildBorderSize  = 1;
	style.PopupBorderSize  = 1;
	style.FrameBorderSize  = is3D;

	style.WindowRounding    = 3;
	style.ChildRounding     = 3;
	style.FrameRounding     = 3;
	style.ScrollbarRounding = 2;
	style.GrabRounding      = 3;

#ifdef IMGUI_HAS_DOCK
	style.TabBorderSize = is3D;
	style.TabRounding   = 3;

	colors[ImGuiCol_DockingEmptyBg]     = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
	colors[ImGuiCol_Tab]                = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	colors[ImGuiCol_TabHovered]         = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
	colors[ImGuiCol_TabActive]          = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
	colors[ImGuiCol_TabUnfocused]       = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
	colors[ImGuiCol_DockingPreview]     = ImVec4(0.85f, 0.85f, 0.85f, 0.28f);
	// clang-format on

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}
#endif
}

}

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

UserInterface::UserInterface(Canvas &canvas, Canvas &resizedCanvas, Canvas &spritesheet, SpriteManager &spriteMgr, AnimationManager &animMgr)
    : canvas_(canvas), resizedCanvas_(resizedCanvas), spritesheet_(spritesheet), spriteMgr_(spriteMgr), animMgr_(animMgr),
      selectedSpriteIndex_(0), spriteGraph_(4)
{
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

#ifdef __ANDROID__
	io.FontGlobalScale = 2.0f;
#endif

#ifdef WITH_FONTAWESOME
	// Merge icons from Font Awesome into the default font
	static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	ImFontConfig icons_config;
	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;
	io.Fonts->AddFontFromFileTTF((nc::IFile::dataPath() + "fonts/" FONT_ICON_FILE_NAME_FAS).data(), 12.0f, &icons_config, icons_ranges);
#endif

	applyDarkStyle();

	spookyLogo_ = nctl::makeUnique<Texture>((nc::IFile::dataPath() + "icon96.png").data());
	ncineLogo_ = nctl::makeUnique<Texture>((nc::IFile::dataPath() + "ncine96.png").data());
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void UserInterface::signalFrameSaved()
{
	ASSERT(shouldSaveFrames_ || shouldSaveSpritesheet_);

	saveAnimStatus_.numSavedFrames++;
	if (shouldSaveFrames_)
		saveAnimStatus_.filename.format("%s_%03d.png", animFilename_.data(), saveAnimStatus_.numSavedFrames);
	else if (shouldSaveSpritesheet_)
	{
		Canvas &sourceCanvas = (saveAnimStatus_.canvasResize != 1.0f) ? resizedCanvas_ : canvas_;
		saveAnimStatus_.sheetDestPos.x += sourceCanvas.texWidth();
		if (saveAnimStatus_.sheetDestPos.x + sourceCanvas.texWidth() > spritesheet_.texWidth())
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
		pushStatusInfoMessage("Animation saved");
	}
}

void UserInterface::pushStatusInfoMessage(const char *message)
{
	statusMessage_.format("%s%s", Labels::INFO_MARKER, message);
	lastStatus_ = nc::TimeStamp::now();
}

void UserInterface::pushStatusErrorMessage(const char *message)
{
	statusMessage_.format("%s%s", Labels::ERROR_MARKER, message);
	lastStatus_ = nc::TimeStamp::now();
}

void UserInterface::closeAboutWindow()
{
	showAboutWindow = false;
}

void UserInterface::cancelRender()
{
	if (shouldSaveFrames_ || shouldSaveSpritesheet_)
	{
		if (shouldSaveFrames_)
			auxString_.format("Render cancelled, saved %d out of %d frames", saveAnimStatus_.numSavedFrames, saveAnimStatus_.numFrames);
		else if (shouldSaveSpritesheet_)
			auxString_ = "Render cancelled, the spritesheet has not been saved";
		pushStatusInfoMessage(auxString_.data());
		saveAnimStatus_.numSavedFrames = 0;
		shouldSaveFrames_ = false;
		shouldSaveSpritesheet_ = false;
	}
}

void UserInterface::removeSelectedSpriteWithKey()
{
	if (hoveringOnCanvas)
		removeSelectedSprite();
}

void UserInterface::moveSprite(int xDiff, int yDiff)
{
	if (spriteMgr_.sprites().isEmpty() == false && hoveringOnCanvas)
	{
		Sprite &sprite = *spriteMgr_.sprites()[selectedSpriteIndex_];
		sprite.x += xDiff;
		sprite.y += yDiff;
	}
}

void UserInterface::menuNew()
{
	// Always clear animations before sprites
	animMgr_.clear();
	spriteMgr_.sprites().clear();
	spriteMgr_.textures().clear();
}

void UserInterface::createGui()
{
	if (lastStatus_.secondsSince() >= 2.0f)
		statusMessage_.clear();

	createDockingSpace();

	ImGui::Begin("SpookyGhost");
	createCanvasGui();
	createSpritesGui();
	createAnimationsGui();
	createRenderGui();
	ImGui::End();

	createCanvasWindow();
	if (spriteMgr_.sprites().isEmpty() == false && showTexrectWindow)
		createTexRectWindow();

	ImGui::Begin("Status");
	ImGui::Text("%s", statusMessage_.data());
	ImGui::End();

	if (showAboutWindow)
		createAboutWindow();
}

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

void UserInterface::createDockingSpace()
{
	ImGuiIO &io = ImGui::GetIO();
	if ((io.ConfigFlags & ImGuiConfigFlags_DockingEnable) == 0)
		return;

	const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
	                                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
	                                     ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;
	ImGuiViewport *viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	bool open = true;
	ImGui::Begin("DockSpace", &open, windowFlags);

	ImGui::PopStyleVar(3);

	createInitialDocking();
	createMenuBar();

	ImGui::End();
}

void UserInterface::createInitialDocking()
{
	const ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_AutoHideTabBar;
	const ImGuiID dockspaceId = ImGui::GetID("TheDockSpace");

	if (ImGui::DockBuilderGetNode(dockspaceId) != nullptr)
	{
		ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), dockspaceFlags);
		return;
	}

	ImGui::DockBuilderRemoveNode(dockspaceId);
	ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);

	ImGuiID dockMainId = dockspaceId;
	ImGuiID dockIdLeft = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Left, 0.3f, nullptr, &dockMainId);
	ImGuiID dockIdDown = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Down, 0.3f, nullptr, &dockMainId);

	ImGui::DockBuilderDockWindow("SpookyGhost", dockIdLeft);
	ImGui::DockBuilderDockWindow("Canvas", dockMainId);
	ImGui::DockBuilderDockWindow("TexRect", dockMainId);
	ImGui::DockBuilderDockWindow("Status", dockIdDown);
	ImGui::DockBuilderFinish(dockspaceId);
}

void UserInterface::createMenuBar()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem(Labels::New, "CTRL + N"))
				menuNew();
			if (ImGui::MenuItem(Labels::Open))
				pushStatusErrorMessage("Open and Save are not implemented yet");
			if (ImGui::MenuItem(Labels::Save))
				pushStatusErrorMessage("Open and Save are not implemented yet");
			if (ImGui::MenuItem(Labels::Quit, "CTRL + Q"))
				nc::theApplication().quit();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("?"))
		{
			if (ImGui::MenuItem(Labels::About))
				showAboutWindow = true;

			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
}

void UserInterface::createCanvasGui()
{
	if (ImGui::CollapsingHeader(Labels::Canvas))
	{
		static int canvasZoomRadio = CanvasZoomEnum::X1;
		canvasZoomRadio = CanvasZoomToEnum(canvasZoom_);
		ImGui::PushID("Canvas");
		ImGui::Text("Zoom:");
		ImGui::SameLine();
		ImGui::RadioButton("1/8x", &canvasZoomRadio, CanvasZoomEnum::X1_8);
		ImGui::SameLine();
		ImGui::RadioButton("1/4x", &canvasZoomRadio, CanvasZoomEnum::X1_4);
		ImGui::SameLine();
		ImGui::RadioButton("1/2x", &canvasZoomRadio, CanvasZoomEnum::X1_2);
		ImGui::SameLine();
		ImGui::RadioButton("1x", &canvasZoomRadio, CanvasZoomEnum::X1);
		ImGui::SameLine();
		ImGui::RadioButton("2x", &canvasZoomRadio, CanvasZoomEnum::X2);
		ImGui::SameLine();
		ImGui::RadioButton("4x", &canvasZoomRadio, CanvasZoomEnum::X4);
		ImGui::SameLine();
		ImGui::RadioButton("8x", &canvasZoomRadio, CanvasZoomEnum::X8);
		ImGui::PopID();
		canvasZoom_ = CanvasEnumToZoom(static_cast<CanvasZoomEnum>(canvasZoomRadio));

		nc::Vector2i desiredCanvasSize;
		static int currentComboResize = static_cast<int>(ResizePresetsEnum::SIZE256); // TODO: hard-coded initial state
		ImGui::Combo("Presets", &currentComboResize, resizePresets, IM_ARRAYSIZE(resizePresets));
		if (currentComboResize == ResizePresetsEnum::CUSTOM)
			ImGui::InputInt2("Custom Size", customCanvasSize_.data());
		else
			customCanvasSize_ = canvas_.size();
		switch (currentComboResize)
		{
			case ResizePresetsEnum::SIZE16:
				desiredCanvasSize.set(16, 16);
				break;
			case ResizePresetsEnum::SIZE32:
				desiredCanvasSize.set(32, 32);
				break;
			case ResizePresetsEnum::SIZE64:
				desiredCanvasSize.set(64, 64);
				break;
			case ResizePresetsEnum::SIZE128:
				desiredCanvasSize.set(128, 128);
				break;
			case ResizePresetsEnum::SIZE256:
				desiredCanvasSize.set(256, 256);
				break;
			case ResizePresetsEnum::SIZE512:
				desiredCanvasSize.set(512, 512);
				break;
			case ResizePresetsEnum::CUSTOM:
				desiredCanvasSize = customCanvasSize_;
				break;
		}

		if (desiredCanvasSize.x < 4)
			desiredCanvasSize.x = 4;
		else if (desiredCanvasSize.x > canvas_.maxTextureSize())
			desiredCanvasSize.x = canvas_.maxTextureSize();
		if (desiredCanvasSize.y < 4)
			desiredCanvasSize.y = 4;
		else if (desiredCanvasSize.y > canvas_.maxTextureSize())
			desiredCanvasSize.y = canvas_.maxTextureSize();

		ImGui::SameLine();
		if (ImGui::Button(Labels::Apply) &&
		    (canvas_.size().x != desiredCanvasSize.x || canvas_.size().y != desiredCanvasSize.y))
		{
			canvas_.resizeTexture(desiredCanvasSize);
		}

		ImGui::Text("Size: %d x %d", canvas_.texWidth(), canvas_.texHeight());
		ImGui::ColorEdit4("Background", canvas_.backgroundColor.data(), ImGuiColorEditFlags_AlphaBar);
	}
}

void UserInterface::createSpritesGui()
{
	static int selectedTextureIndex = 0;
	if (ImGui::CollapsingHeader(Labels::Sprites))
	{
		if (spriteMgr_.textures().isEmpty() == false)
		{
			comboString_.clear();
			for (unsigned int i = 0; i < spriteMgr_.textures().size(); i++)
			{
				Texture &texture = *spriteMgr_.textures()[i];
				comboString_.formatAppend("#%u: \"%s\" (%d x %d)", i, texture.name().data(), texture.width(), texture.height());
				comboString_.setLength(comboString_.length() + 1);
			}
			comboString_.setLength(comboString_.length() + 1);
			// Append a second '\0' to signal the end of the combo item list
			comboString_[comboString_.length() - 1] = '\0';

			ImGui::Combo("Texture", &selectedTextureIndex, comboString_.data());

			ImGui::SameLine();
			auxString_.format("%s##Texture", Labels::Remove);
			if (ImGui::Button(auxString_.data()))
			{
				for (unsigned int i = 0; i < spriteMgr_.sprites().size(); i++)
				{
					Sprite &sprite = *spriteMgr_.sprites()[i];
					if (&sprite.texture() == spriteMgr_.textures()[selectedTextureIndex].get())
					{
						animMgr_.removeSprite(&sprite);
						spriteMgr_.sprites().removeAt(i);
					}
				}
				spriteMgr_.textures().removeAt(selectedTextureIndex);
				if (selectedTextureIndex > 0)
					selectedTextureIndex--;
			}
		}
		ImGui::InputText("Filename", texFilename_.data(), MaxStringLength,
		                 ImGuiInputTextFlags_CallbackResize, inputTextCallback, &texFilename_);
		ImGui::SameLine();
		if (ImGui::Button(Labels::Load) && texFilename_.isEmpty() == false)
		{
			if (nc::IFile::access((nc::IFile::dataPath() + texFilename_).data(), nc::IFile::AccessMode::READABLE))
			{
				spriteMgr_.textures().pushBack(nctl::makeUnique<Texture>((nc::IFile::dataPath() + texFilename_).data()));
				auxString_.format("Loaded texture \"%s\"", (nc::IFile::dataPath() + texFilename_).data());
				selectedTextureIndex = spriteMgr_.textures().size() - 1;
			}
			else if (nc::IFile::access(texFilename_.data(), nc::IFile::AccessMode::READABLE))
			{
				spriteMgr_.textures().pushBack(nctl::makeUnique<Texture>(texFilename_.data()));
				auxString_.format("Loaded texture \"%s\"", texFilename_.data());
				selectedTextureIndex = spriteMgr_.textures().size() - 1;
			}
			else
				auxString_.format("Cannot load texture \"%s\"", texFilename_.data());

			pushStatusInfoMessage(auxString_.data());
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		if (spriteMgr_.textures().isEmpty() == false)
		{
			if (ImGui::Button(Labels::Add))
			{
				if (selectedTextureIndex >= 0 && selectedTextureIndex < spriteMgr_.textures().size())
				{
					Texture &tex = *spriteMgr_.textures()[selectedTextureIndex];
					spriteMgr_.sprites().pushBack(nctl::makeUnique<Sprite>(&tex));
					selectedSpriteIndex_ = spriteMgr_.sprites().size() - 1;
				}
			}
			ImGui::SameLine();
			if (ImGui::Button(Labels::Remove))
				removeSelectedSprite();
		}
		else
			ImGui::Text("Load at least one texture in order to add sprites");

		if (spriteMgr_.sprites().isEmpty() == false)
		{
			if (selectedSpriteIndex_ > 0)
			{
				ImGui::SameLine();
				if (ImGui::Button(Labels::MoveUp))
				{
					nctl::swap(spriteMgr_.sprites()[selectedSpriteIndex_], spriteMgr_.sprites()[selectedSpriteIndex_ - 1]);
					selectedSpriteIndex_--;
				}
			}
			if (selectedSpriteIndex_ < spriteMgr_.sprites().size() - 1)
			{
				ImGui::SameLine();
				if (ImGui::Button(Labels::MoveDown))
				{
					nctl::swap(spriteMgr_.sprites()[selectedSpriteIndex_], spriteMgr_.sprites()[selectedSpriteIndex_ + 1]);
					selectedSpriteIndex_++;
				}
			}

			comboString_.clear();
			for (unsigned int i = 0; i < spriteMgr_.sprites().size(); i++)
			{
				Sprite &sprite = *spriteMgr_.sprites()[i];
				comboString_.formatAppend("#%u: \"%s\" (%d x %d)", i, sprite.name.data(), sprite.width(), sprite.height());
				comboString_.setLength(comboString_.length() + 1);
			}
			comboString_.setLength(comboString_.length() + 1);
			// Append a second '\0' to signal the end of the combo item list
			comboString_[comboString_.length() - 1] = '\0';

			ImGui::Combo("Sprite", &selectedSpriteIndex_, comboString_.data());

			Sprite &sprite = *spriteMgr_.sprites()[selectedSpriteIndex_];

			Texture &tex = sprite.texture();
			ImGui::Text("Texture: %s (%dx%d)", tex.name().data(), tex.width(), tex.height());

			ImGui::InputText("Name", sprite.name.data(), Sprite::MaxNameLength,
			                 ImGuiInputTextFlags_CallbackResize, inputTextCallback, &sprite.name);
			ImGui::SameLine();
			ImGui::Checkbox("Visible", &sprite.visible);

			// Create an array of sprites that can be a parent of the selected one
			spriteGraph_.clear();
			spriteGraph_.pushBack(SpriteStruct(-1, nullptr));
			for (unsigned int i = 0; i < spriteMgr_.sprites().size(); i++)
				spriteMgr_.sprites()[i]->visited = false;
			visitSprite(sprite);
			for (unsigned int i = 0; i < spriteMgr_.sprites().size(); i++)
			{
				if (spriteMgr_.sprites()[i]->visited == false)
					spriteGraph_.pushBack(SpriteStruct(i, spriteMgr_.sprites()[i].get()));
			}

			int currentParentCombo = 0; // None
			comboString_.clear();
			for (unsigned int i = 0; i < spriteGraph_.size(); i++)
			{
				const int index = spriteGraph_[i].index;
				const Sprite &currentSprite = *spriteGraph_[i].sprite;
				if (index < 0)
					comboString_.append("None");
				else
					comboString_.formatAppend("#%u: \"%s\" (%d x %d)", index, currentSprite.name.data(), currentSprite.width(), currentSprite.height());
				comboString_.setLength(comboString_.length() + 1);

				if (sprite.parent() == &currentSprite)
					currentParentCombo = i;
			}
			comboString_.setLength(comboString_.length() + 1);
			// Append a second '\0' to signal the end of the combo item list
			comboString_[comboString_.length() - 1] = '\0';

			ImGui::Combo("Parent", &currentParentCombo, comboString_.data());

			const Sprite *prevParent = sprite.parent();
			const nc::Vector2f absPosition = sprite.absPosition();
			Sprite *parent = spriteGraph_[currentParentCombo].sprite;
			if (prevParent != parent)
			{
				sprite.setParent(parent);
				sprite.setAbsPosition(absPosition);
			}

			nc::Vector2f position(sprite.x, sprite.y);
			ImGui::SliderFloat2("Position", position.data(), 0.0f, static_cast<float>(canvas_.texWidth()));
			sprite.x = roundf(position.x);
			sprite.y = roundf(position.y);
			ImGui::SliderFloat("Rotation", &sprite.rotation, 0.0f, 360.0f);
			ImGui::SliderFloat2("Scale", sprite.scaleFactor.data(), 0.0f, 8.0f);
			ImGui::SameLine();
			auxString_.format("%s##Scale", Labels::Reset);
			if (ImGui::Button(auxString_.data()))
				sprite.scaleFactor.set(1.0f, 1.0f);

			const float halfBiggerDimension = sprite.width() > sprite.height() ? sprite.width() * 0.5f : sprite.height() * 0.5f;
			ImGui::SliderFloat2("Anchor Point", sprite.anchorPoint.data(), -halfBiggerDimension, halfBiggerDimension);
			static int currentAnchorSelection = 0;
			if (ImGui::Combo("Anchor Presets", &currentAnchorSelection, anchorPointItems, IM_ARRAYSIZE(anchorPointItems)))
			{
				switch (currentAnchorSelection)
				{
					case AnchorPointsEnum::CENTER:
						sprite.anchorPoint.set(0.0f, 0.0f);
						break;
					case AnchorPointsEnum::BOTTOM_LEFT:
						sprite.anchorPoint.set(-sprite.width() * 0.5f, sprite.height() * 0.5f);
						break;
					case AnchorPointsEnum::TOP_LEFT:
						sprite.anchorPoint.set(-sprite.width() * 0.5f, -sprite.height() * 0.5f);
						break;
					case AnchorPointsEnum::BOTTOM_RIGHT:
						sprite.anchorPoint.set(sprite.width() * 0.5f, sprite.height() * 0.5f);
						break;
					case AnchorPointsEnum::TOP_RIGHT:
						sprite.anchorPoint.set(sprite.width() * 0.5f, -sprite.height() * 0.5f);
						break;
				}
			}
			if (sprite.parent() != nullptr)
			{
				ImGui::Text("Abs Position: %f, %f", sprite.absPosition().x, sprite.absPosition().y);
				ImGui::Text("Abs Rotation: %f", sprite.absRotation());
				ImGui::Text("Abs Scale: %f, %f", sprite.absScaleFactor().x, sprite.absScaleFactor().y);
			}

			ImGui::Separator();
			nc::Recti texRect = sprite.texRect();
			int minX = texRect.x;
			int maxX = minX + texRect.w;
			ImGui::DragIntRange2("Rect X", &minX, &maxX, 1.0f, 0, tex.width());

			ImGui::SameLine();
			ImGui::Checkbox("Show Preview", &showTexrectWindow);

			int minY = texRect.y;
			int maxY = minY + texRect.h;
			ImGui::DragIntRange2("Rect Y", &minY, &maxY, 1.0f, 0, tex.height());

			texRect.x = minX;
			texRect.w = maxX - minX;
			texRect.y = minY;
			texRect.h = maxY - minY;
			ImGui::SameLine();
			auxString_.format("%s##Rect", Labels::Reset);
			if (ImGui::Button(auxString_.data()))
				texRect = nc::Recti(0, 0, tex.width(), tex.height());

			const nc::Recti currentTexRect = sprite.texRect();
			if (texRect.x != currentTexRect.x || texRect.y != currentTexRect.y ||
			    texRect.w != currentTexRect.w || texRect.h != currentTexRect.h)
				sprite.setTexRect(texRect);

			bool isFlippedX = sprite.isFlippedX();
			ImGui::Checkbox("Flipped X", &isFlippedX);
			ImGui::SameLine();
			bool isFlippedY = sprite.isFlippedY();
			ImGui::Checkbox("Flipped Y", &isFlippedY);

			if (isFlippedX != sprite.isFlippedX())
				sprite.setFlippedX(isFlippedX);
			if (isFlippedY != sprite.isFlippedY())
				sprite.setFlippedY(isFlippedY);

			ImGui::Separator();
			int currentBlendingPreset = static_cast<int>(sprite.blendingPreset());
			ImGui::Combo("Blending", &currentBlendingPreset, blendingPresets, IM_ARRAYSIZE(blendingPresets));
			sprite.setBlendingPreset(static_cast<Sprite::BlendingPreset>(currentBlendingPreset));

			ImGui::ColorEdit4("Color", sprite.color.data(), ImGuiColorEditFlags_AlphaBar);
			ImGui::SameLine();
			auxString_.format("%s##Color", Labels::Reset);
			if (ImGui::Button(auxString_.data()))
				sprite.color = nc::Colorf::White;
		}
	}
}

void UserInterface::createAnimationStateGui(IAnimation &anim)
{
	ImGui::Separator();
	ImGui::Text("State: %s", animStateToString(anim.state()));
	if (ImGui::Button(Labels::Stop))
		anim.stop();
	ImGui::SameLine();
	if (ImGui::Button(Labels::Pause))
		anim.pause();
	ImGui::SameLine();
	if (ImGui::Button(Labels::Play))
		anim.play();
}

void UserInterface::createCurveAnimationGui(CurveAnimation &anim, const CurveAnimationGuiLimits &limits)
{
	int currentComboCurveType = static_cast<int>(anim.curve().type());
	ImGui::Combo("Easing Curve", &currentComboCurveType, easingCurveTypes, IM_ARRAYSIZE(easingCurveTypes));
	anim.curve().setType(static_cast<EasingCurve::Type>(currentComboCurveType));

	int currentDirectionMode = static_cast<int>(anim.curve().direction());
	ImGui::Combo("Direction", &currentDirectionMode, easingCurveDirections, IM_ARRAYSIZE(easingCurveDirections));
	anim.curve().setDirection(static_cast<EasingCurve::Direction>(currentDirectionMode));

	int currentComboLoopMode = static_cast<int>(anim.curve().loopMode());
	ImGui::Combo("Loop Mode", &currentComboLoopMode, easingCurveLoopModes, IM_ARRAYSIZE(easingCurveLoopModes));
	anim.curve().setLoopMode(static_cast<EasingCurve::LoopMode>(currentComboLoopMode));

	ImGui::SliderFloat("Shift", &anim.curve().shift(), limits.minShift, limits.maxShift);
	ImGui::SameLine();
	auxString_.format("%s##Shift", Labels::Reset);
	if (ImGui::Button(auxString_.data()))
		anim.curve().shift() = 0.0f;
	ImGui::SliderFloat("Scale", &anim.curve().scale(), limits.minScale, limits.maxScale);
	ImGui::SameLine();
	auxString_.format("%s##Scale", Labels::Reset);
	if (ImGui::Button(auxString_.data()))
		anim.curve().scale() = 1.0f;

	ImGui::Separator();
	ImGui::SliderFloat("Speed", &anim.speed(), 0.0f, 5.0f);
	ImGui::SameLine();
	auxString_.format("%s##Speed", Labels::Reset);
	if (ImGui::Button(auxString_.data()))
		anim.speed() = 1.0f;
	ImGui::SliderFloat("Start", &anim.curve().start(), 0.0f, 1.0f);
	ImGui::SliderFloat("End", &anim.curve().end(), 0.0f, 1.0f);
	ImGui::SliderFloat("Time", &anim.curve().time(), anim.curve().start(), anim.curve().end());

	if (anim.curve().start() > anim.curve().end() ||
	    anim.curve().end() < anim.curve().start())
	{
		anim.curve().start() = anim.curve().end();
	}
	if (anim.curve().time() < anim.curve().start())
		anim.curve().time() = anim.curve().start();
	else if (anim.curve().time() > anim.curve().end())
		anim.curve().time() = anim.curve().end();

	ImGui::Text("Value: %f", anim.curve().value());
}

void UserInterface::createAnimationRemoveButton(AnimationGroup &parentGroup, unsigned int index)
{
	if (ImGui::Button(Labels::Remove))
	{
		parentGroup.anims()[index]->stop();
		parentGroup.anims().removeAt(index);
	}
}

void UserInterface::createAnimationsGui()
{
	if (ImGui::CollapsingHeader(Labels::Animations))
	{
		static int currentComboAnimType = 0;
		ImGui::Combo("Type", &currentComboAnimType, animationTypes, IM_ARRAYSIZE(animationTypes));
		ImGui::SameLine();
		auxString_.format("%s##Animations", Labels::Add);
		if (ImGui::Button(auxString_.data()))
		{
			switch (currentComboAnimType)
			{
				case AnimationTypesEnum::PARALLEL_GROUP:
					animMgr_.anims().pushBack(nctl::makeUnique<ParallelAnimationGroup>());
					break;
				case AnimationTypesEnum::SEQUENTIAL_GROUP:
					animMgr_.anims().pushBack(nctl::makeUnique<SequentialAnimationGroup>());
					break;
				case AnimationTypesEnum::PROPERTY:
					animMgr_.anims().pushBack(nctl::makeUnique<PropertyAnimation>());
					break;
				case AnimationTypesEnum::GRID:
					animMgr_.anims().pushBack(nctl::makeUnique<GridAnimation>());
					break;
			}
		}

		ImGui::SameLine();
		auxString_.format("%s##Animations", Labels::Clear);
		if (ImGui::Button(auxString_.data()))
			animMgr_.clear();
		ImGui::Separator();

		for (unsigned int i = 0; i < animMgr_.anims().size(); i++)
			createRecursiveAnimationsGui(animMgr_.animGroup(), i);
	}
}

void UserInterface::createRenderGui()
{
	if (ImGui::CollapsingHeader(Labels::Render))
	{
		ImGui::InputText("Filename prefix", animFilename_.data(), MaxStringLength,
		                 ImGuiInputTextFlags_CallbackResize, inputTextCallback, &animFilename_);

		static int canvasResizeRadio = CanvasZoomEnum::X1;
		ImGui::PushID("Render");
		ImGui::Text("Resize:");
		ImGui::SameLine();
		ImGui::RadioButton("1/8x", &canvasResizeRadio, CanvasZoomEnum::X1_8);
		ImGui::SameLine();
		ImGui::RadioButton("1/4x", &canvasResizeRadio, CanvasZoomEnum::X1_4);
		ImGui::SameLine();
		ImGui::RadioButton("1/2x", &canvasResizeRadio, CanvasZoomEnum::X1_2);
		ImGui::SameLine();
		ImGui::RadioButton("1x", &canvasResizeRadio, CanvasZoomEnum::X1);
		ImGui::SameLine();
		ImGui::RadioButton("2x", &canvasResizeRadio, CanvasZoomEnum::X2);
		ImGui::SameLine();
		ImGui::RadioButton("4x", &canvasResizeRadio, CanvasZoomEnum::X4);
		ImGui::SameLine();
		ImGui::RadioButton("8x", &canvasResizeRadio, CanvasZoomEnum::X8);
		ImGui::PopID();
		saveAnimStatus_.canvasResize = CanvasEnumToZoom(static_cast<CanvasZoomEnum>(canvasResizeRadio));

		ImGui::InputInt("FPS", &saveAnimStatus_.fps);
		ImGui::SliderInt("Num Frames", &saveAnimStatus_.numFrames, 1, 10 * saveAnimStatus_.fps); // Hard-coded limit
		float duration = saveAnimStatus_.numFrames * saveAnimStatus_.inverseFps();
		ImGui::SliderFloat("Duration", &duration, 0.0f, 10.0f, "%.3fs"); // Hard-coded limit

		const nc::Vector2i uncappedFrameSize(canvas_.texWidth() * saveAnimStatus_.canvasResize, canvas_.texHeight() * saveAnimStatus_.canvasResize);
		// Immediately-invoked function expression for const initialization
		const nc::Vector2i frameSize = [&] {
			nc::Vector2i size = uncappedFrameSize;
			size.x = (size.x > canvas_.maxTextureSize()) ? canvas_.maxTextureSize() : size.x;
			size.y = (size.y > canvas_.maxTextureSize()) ? canvas_.maxTextureSize() : size.y;
			return size;
		}();

		auxString_.format("%d x %d", frameSize.x, frameSize.y);
		if (uncappedFrameSize.x > frameSize.x || uncappedFrameSize.y > frameSize.y)
			auxString_.formatAppend(" (capped from %d x %d)", uncappedFrameSize.x, uncappedFrameSize.y);
		ImGui::Text("Frame size: %s", auxString_.data());

		const int sideX = static_cast<int>(ceil(sqrt(saveAnimStatus_.numFrames)));
		const int sideY = (sideX * (sideX - 1) > saveAnimStatus_.numFrames) ? sideX - 1 : sideX;
		const nc::Vector2i uncappedSpritesheetSize(sideX * frameSize.x, sideY * frameSize.y);
		// Immediately-invoked function expression for const initialization
		const nc::Vector2i spritesheetSize = [&] {
			nc::Vector2i size = uncappedSpritesheetSize;
			size.x = (size.x > canvas_.maxTextureSize()) ? canvas_.maxTextureSize() : size.x;
			size.y = (size.y > canvas_.maxTextureSize()) ? canvas_.maxTextureSize() : size.y;
			return size;
		}();

		auxString_.format("%d x %d", spritesheetSize.x, spritesheetSize.y);
		if (uncappedSpritesheetSize.x > spritesheetSize.x || uncappedSpritesheetSize.y > spritesheetSize.y)
			auxString_.formatAppend(" (capped from %d x %d)", uncappedSpritesheetSize.x, uncappedSpritesheetSize.y);
		ImGui::Text("Spritesheet size: %s", auxString_.data());

		saveAnimStatus_.numFrames = static_cast<int>(duration * saveAnimStatus_.fps);
		if (saveAnimStatus_.numFrames < 1)
			saveAnimStatus_.numFrames = 1;

		if (shouldSaveFrames_ || shouldSaveSpritesheet_)
		{
			const unsigned int numSavedFrames = saveAnimStatus_.numSavedFrames;
			const float fraction = numSavedFrames / static_cast<float>(saveAnimStatus_.numFrames);
			auxString_.format("Frame: %d/%d", numSavedFrames, saveAnimStatus_.numFrames);
			ImGui::ProgressBar(fraction, ImVec2(0.0f, 0.0f), auxString_.data());
			ImGui::SameLine();
			if (ImGui::Button(Labels::Cancel))
				cancelRender();
		}
		else
		{
			if (ImGui::Button(Labels::SaveFrames))
			{
				if (animFilename_.isEmpty())
					pushStatusErrorMessage("Set a filename prefix before saving an animation");
				else
				{
					animMgr_.play();
					saveAnimStatus_.filename.format("%s_%03d.png", animFilename_.data(), saveAnimStatus_.numSavedFrames);
					shouldSaveFrames_ = true;
					resizedCanvas_.resizeTexture(frameSize);
				}
			}
			ImGui::SameLine();
			if (ImGui::Button(Labels::SaveSpritesheet))
			{
				if (animFilename_.isEmpty())
					pushStatusErrorMessage("Set a filename prefix before saving an animation");
				else
				{
					animMgr_.play();
					saveAnimStatus_.filename.format("%s.png", animFilename_.data(), saveAnimStatus_.numSavedFrames);
					shouldSaveSpritesheet_ = true;
					saveAnimStatus_.sheetDestPos.set(0, 0);
					resizedCanvas_.resizeTexture(canvas_.size() * saveAnimStatus_.canvasResize);
					spritesheet_.resizeTexture(spritesheetSize);
				}
			}
		}
	}
}

void UserInterface::createRecursiveAnimationsGui(AnimationGroup &parentGroup, unsigned int index)
{
	IAnimation &anim = *parentGroup.anims()[index];

	ImGui::PushID(reinterpret_cast<const void *>(&anim));
	switch (anim.type())
	{
		case IAnimation::Type::PROPERTY:
		{
			createPropertyAnimationGui(parentGroup, index);
			break;
		}
		case IAnimation::Type::GRID:
		{
			createGridAnimationGui(parentGroup, index);
			break;
		}
		case IAnimation::Type::PARALLEL_GROUP:
		{
			createAnimationGroupGui(parentGroup, index);
			break;
		}
		case IAnimation::Type::SEQUENTIAL_GROUP:
		{
			createAnimationGroupGui(parentGroup, index);
			break;
		}
	}
	ImGui::PopID();
}

void UserInterface::createAnimationGroupGui(AnimationGroup &parentGroup, unsigned int index)
{
	AnimationGroup &animGroup = static_cast<AnimationGroup &>(*parentGroup.anims()[index]);
	ASSERT(animGroup.type() == IAnimation::Type::PARALLEL_GROUP ||
	       animGroup.type() == IAnimation::Type::SEQUENTIAL_GROUP);

	auxString_.format("#%u: ", index);
	if (animGroup.name.isEmpty() == false)
		auxString_.formatAppend("\"%s\" (", animGroup.name.data());
	if (animGroup.type() == IAnimation::Type::PARALLEL_GROUP)
		auxString_.append("Parallel Animation");
	else if (animGroup.type() == IAnimation::Type::SEQUENTIAL_GROUP)
		auxString_.append("Sequential Animation");
	if (animGroup.name.isEmpty() == false)
		auxString_.append(")");
	auxString_.append("###AnimationGroup");

	if (ImGui::TreeNodeEx(auxString_.data(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::InputText("Name", animGroup.name.data(), IAnimation::MaxNameLength,
		                 ImGuiInputTextFlags_CallbackResize, inputTextCallback, &animGroup.name);

		static int currentComboAnimType = 0;
		ImGui::Combo("Type", &currentComboAnimType, animationTypes, IM_ARRAYSIZE(animationTypes));
		ImGui::SameLine();
		if (ImGui::Button(Labels::Add))
		{
			switch (currentComboAnimType)
			{
				case AnimationTypesEnum::PARALLEL_GROUP:
				{
					animGroup.anims().pushBack(nctl::makeUnique<ParallelAnimationGroup>());
					break;
				}
				case AnimationTypesEnum::SEQUENTIAL_GROUP:
				{
					animGroup.anims().pushBack(nctl::makeUnique<SequentialAnimationGroup>());
					break;
				}
				case AnimationTypesEnum::PROPERTY:
				{
					animGroup.anims().pushBack(nctl::makeUnique<PropertyAnimation>());
					break;
				}
				case AnimationTypesEnum::GRID:
				{
					animGroup.anims().pushBack(nctl::makeUnique<GridAnimation>());
					break;
				}
			}
		}
		ImGui::SameLine();
		if (ImGui::Button(Labels::Clear))
		{
			animGroup.stop();
			animGroup.anims().clear();
		}

		createAnimationStateGui(animGroup);
		createAnimationRemoveButton(parentGroup, index);

		for (unsigned int i = 0; i < animGroup.anims().size(); i++)
			createRecursiveAnimationsGui(animGroup, i);

		ImGui::TreePop();
	}
}

void UserInterface::createPropertyAnimationGui(AnimationGroup &parentGroup, unsigned int index)
{
	PropertyAnimation &anim = static_cast<PropertyAnimation &>(*parentGroup.anims()[index]);
	ASSERT(anim.type() == IAnimation::Type::PROPERTY);

	auxString_.format("#%u: ", index);
	if (anim.name.isEmpty() == false)
		auxString_.formatAppend("\"%s\" (", anim.name.data());
	auxString_.formatAppend("%s property", anim.propertyName().data());
	if (anim.sprite() != nullptr && anim.sprite()->name.isEmpty() == false)
		auxString_.formatAppend(" for sprite \"%s\"", anim.sprite()->name.data());
	if (anim.name.isEmpty() == false)
		auxString_.append(")");
	auxString_.append("###PropertyAnimation");

	static CurveAnimationGuiLimits limits;
	if (ImGui::TreeNodeEx(auxString_.data(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::InputText("Name", anim.name.data(), IAnimation::MaxNameLength,
		                 ImGuiInputTextFlags_CallbackResize, inputTextCallback, &anim.name);
		ImGui::Separator();

		int spriteIndex = spriteMgr_.spriteIndex(anim.sprite());
		if (spriteMgr_.sprites().isEmpty() == false)
		{
			if (spriteIndex < 0)
				spriteIndex = selectedSpriteIndex_;

			comboString_.clear();
			for (unsigned int i = 0; i < spriteMgr_.sprites().size(); i++)
			{
				Sprite &sprite = *spriteMgr_.sprites()[i];
				comboString_.formatAppend("#%u: %s (%d x %d)", i, sprite.name.data(), sprite.width(), sprite.height());
				comboString_.setLength(comboString_.length() + 1);
			}
			comboString_.setLength(comboString_.length() + 1);
			// Append a second '\0' to signal the end of the combo item list
			comboString_[comboString_.length() - 1] = '\0';

			ImGui::Combo("Sprite", &spriteIndex, comboString_.data());
			anim.setSprite(spriteMgr_.sprites()[spriteIndex].get());

			Sprite &sprite = *spriteMgr_.sprites()[spriteIndex];

			static int currentComboProperty = -1;
			const nctl::String &propertyName = anim.propertyName();
			currentComboProperty = PropertyTypesEnum::NONE;
			if (anim.property() != nullptr)
			{
				for (unsigned int i = 0; i < IM_ARRAYSIZE(propertyTypes); i++)
				{
					if (propertyName == propertyTypes[i])
					{
						currentComboProperty = static_cast<PropertyTypesEnum>(i);
						break;
					}
				}
			}

			bool setCurveShift = false;
			if (ImGui::Combo("Property", &currentComboProperty, propertyTypes, IM_ARRAYSIZE(propertyTypes)))
				setCurveShift = true;
			anim.setPropertyName(propertyTypes[currentComboProperty]);
			switch (currentComboProperty)
			{
				case PropertyTypesEnum::NONE:
					anim.setProperty(nullptr);
					break;
				case PropertyTypesEnum::POSITION_X:
					anim.setProperty(&sprite.x);
					limits.minShift = -sprite.width() * 0.5f;
					limits.maxShift = canvas_.texWidth() + sprite.width() * 0.5f;
					limits.minScale = -canvas_.texWidth();
					limits.maxScale = canvas_.texWidth();
					break;
				case PropertyTypesEnum::POSITION_Y:
					anim.setProperty(&sprite.y);
					limits.minShift = -sprite.height() * 0.5f;
					limits.maxShift = canvas_.texHeight() + sprite.height() * 0.5f;
					limits.minScale = -canvas_.texHeight();
					limits.maxScale = canvas_.texHeight();
					break;
				case PropertyTypesEnum::ROTATION:
					anim.setProperty(&sprite.rotation);
					limits.minShift = 0.0f;
					limits.maxShift = 360.0f;
					limits.minScale = -360.0f;
					limits.maxScale = 360.0f;
					break;
				case PropertyTypesEnum::SCALE_X:
					anim.setProperty(&sprite.scaleFactor.x);
					limits.minShift = -8.0f;
					limits.maxShift = 8.0f;
					limits.minScale = -8.0f;
					limits.maxScale = 8.0f;
					break;
				case PropertyTypesEnum::SCALE_Y:
					anim.setProperty(&sprite.scaleFactor.y);
					limits.minShift = -8.0f;
					limits.maxShift = 8.0f;
					limits.minScale = -8.0f;
					limits.maxScale = 8.0f;
					break;
				case PropertyTypesEnum::ANCHOR_X:
					anim.setProperty(&sprite.anchorPoint.x);
					limits.minShift = -sprite.width() * 0.5f;
					limits.maxShift = sprite.width() * 0.5f;
					limits.minScale = -sprite.width();
					limits.maxScale = sprite.width();
					break;
				case PropertyTypesEnum::ANCHOR_Y:
					anim.setProperty(&sprite.anchorPoint.y);
					limits.minShift = -sprite.height() * 0.5f;
					limits.maxShift = sprite.height() * 0.5f;
					limits.minScale = -sprite.height();
					limits.maxScale = sprite.height();
					break;
				case PropertyTypesEnum::OPACITY:
					anim.setProperty(&sprite.color.data()[3]);
					limits.minShift = 0.0f;
					limits.maxShift = 1.0f;
					limits.minScale = -1.0f;
					limits.maxScale = 1.0f;
					break;
				case PropertyTypesEnum::COLOR_R:
					anim.setProperty(&sprite.color.data()[0]);
					limits.minShift = 0.0f;
					limits.maxShift = 1.0f;
					limits.minScale = -1.0f;
					limits.maxScale = 1.0f;
					break;
				case PropertyTypesEnum::COLOR_G:
					anim.setProperty(&sprite.color.data()[1]);
					limits.minShift = 0.0f;
					limits.maxShift = 1.0f;
					limits.minScale = -1.0f;
					limits.maxScale = 1.0f;
					break;
				case PropertyTypesEnum::COLOR_B:
					anim.setProperty(&sprite.color.data()[2]);
					limits.minShift = 0.0f;
					limits.maxShift = 1.0f;
					limits.minScale = -1.0f;
					limits.maxScale = 1.0f;
					break;
			}
			if (setCurveShift && anim.property())
				anim.curve().setShift(*anim.property());
		}
		else
			ImGui::TextDisabled("There are no sprites to animate");

		createCurveAnimationGui(anim, limits);
		createAnimationStateGui(anim);
		createAnimationRemoveButton(parentGroup, index);

		ImGui::TreePop();
	}
}

void UserInterface::createGridAnimationGui(AnimationGroup &parentGroup, unsigned int index)
{
	GridAnimation &anim = static_cast<GridAnimation &>(*parentGroup.anims()[index]);
	ASSERT(anim.type() == IAnimation::Type::GRID);

	auxString_.format("#%u: ", index);
	if (anim.name.isEmpty() == false)
		auxString_.formatAppend("\"%s\" (", anim.name.data());
	if (anim.function() != nullptr)
		auxString_.formatAppend("%s grid", anim.function()->name().data());
	if (anim.sprite() != nullptr && anim.sprite()->name.isEmpty() == false)
		auxString_.formatAppend(" for sprite \"%s\"", anim.sprite()->name.data());
	if (anim.name.isEmpty() == false)
		auxString_.append(")");
	auxString_.append("###GridAnimation");

	CurveAnimationGuiLimits limits;
	limits.minScale = -10.0f;
	limits.maxScale = 10.0f;
	limits.minShift = -100.0f;
	limits.maxShift = 100.0f;
	if (ImGui::TreeNodeEx(auxString_.data(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::InputText("Name", anim.name.data(), IAnimation::MaxNameLength,
		                 ImGuiInputTextFlags_CallbackResize, inputTextCallback, &anim.name);
		ImGui::Separator();

		int spriteIndex = spriteMgr_.spriteIndex(anim.sprite());
		if (spriteMgr_.sprites().isEmpty() == false)
		{
			if (spriteIndex < 0)
				spriteIndex = selectedSpriteIndex_;

			comboString_.clear();
			for (unsigned int i = 0; i < spriteMgr_.sprites().size(); i++)
			{
				Sprite &sprite = *spriteMgr_.sprites()[i];
				comboString_.formatAppend("#%u: %s (%d x %d)", i, sprite.name.data(), sprite.width(), sprite.height());
				comboString_.setLength(comboString_.length() + 1);
			}
			comboString_.setLength(comboString_.length() + 1);
			// Append a second '\0' to signal the end of the combo item list
			comboString_[comboString_.length() - 1] = '\0';

			ImGui::Combo("Sprite", &spriteIndex, comboString_.data());
			Sprite *sprite = spriteMgr_.sprites()[spriteIndex].get();
			if (anim.sprite() != sprite)
				anim.setSprite(sprite);
		}
		else
			ImGui::TextDisabled("There are no sprites to animate");

		static int currentComboFunction = -1;
		comboString_.clear();
		for (unsigned int i = 0; i < GridFunctionLibrary::gridFunctions().size(); i++)
		{
			const nctl::String &functionName = GridFunctionLibrary::gridFunctions()[i].name();
			comboString_.formatAppend("%s", functionName.data());
			comboString_.setLength(comboString_.length() + 1);

			if (anim.function() && functionName == anim.function()->name())
				currentComboFunction = i;
		}
		comboString_.setLength(comboString_.length() + 1);
		// Append a second '\0' to signal the end of the combo item list
		comboString_[comboString_.length() - 1] = '\0';
		ASSERT(currentComboFunction > -1);

		ImGui::Combo("Function", &currentComboFunction, comboString_.data());
		const GridFunction *gridFunction = &GridFunctionLibrary::gridFunctions()[currentComboFunction];
		if (anim.function() != gridFunction)
			anim.setFunction(gridFunction);

		if (anim.function() != nullptr)
		{
			for (unsigned int i = 0; i < anim.function()->numParameters(); i++)
			{
				const GridFunction::ParameterInfo &paramInfo = anim.function()->parameterInfo(i);
				auxString_.format("%s##GridFunction%u", paramInfo.name.data(), i);
				float minValue = paramInfo.minValue.value0;
				float maxValue = paramInfo.maxValue.value0;

				if (anim.sprite())
				{
					if (paramInfo.minMultiply == GridFunction::ValueMultiply::SPRITE_WIDTH)
						minValue *= anim.sprite()->width();
					else if (paramInfo.minMultiply == GridFunction::ValueMultiply::SPRITE_HEIGHT)
						minValue *= anim.sprite()->height();

					if (paramInfo.maxMultiply == GridFunction::ValueMultiply::SPRITE_WIDTH)
						maxValue *= anim.sprite()->width();
					else if (paramInfo.maxMultiply == GridFunction::ValueMultiply::SPRITE_HEIGHT)
						maxValue *= anim.sprite()->height();
				}

				switch (paramInfo.type)
				{
					case GridFunction::ParameterType::FLOAT:
						ImGui::SliderFloat(auxString_.data(), &anim.parameters()[i].value0, minValue, maxValue);
						break;
					case GridFunction::ParameterType::VECTOR2F:
						ImGui::SliderFloat2(auxString_.data(), &anim.parameters()[i].value0, minValue, maxValue);
						break;
				}

				if (anim.sprite())
				{
					if (paramInfo.anchorType == GridFunction::AnchorType::X)
						anim.sprite()->gridAnchorPoint.x = anim.parameters()[i].value0;
					else if (paramInfo.anchorType == GridFunction::AnchorType::Y)
						anim.sprite()->gridAnchorPoint.y = anim.parameters()[i].value0;
					else if (paramInfo.anchorType == GridFunction::AnchorType::XY)
					{
						anim.sprite()->gridAnchorPoint.x = anim.parameters()[i].value0;
						anim.sprite()->gridAnchorPoint.y = anim.parameters()[i].value1;
					}
				}

				ImGui::SameLine();
				auxString_.format("%s##GridFunction%u", Labels::Reset, i);
				if (ImGui::Button(auxString_.data()))
				{
					anim.parameters()[i].value0 = paramInfo.initialValue.value0;
					anim.parameters()[i].value1 = paramInfo.initialValue.value1;
				}
			}
		}

		createCurveAnimationGui(anim, limits);
		createAnimationStateGui(anim);
		createAnimationRemoveButton(parentGroup, index);

		ImGui::TreePop();
	}
}

void UserInterface::SpriteProperties::save(Sprite &sprite)
{
	if (saved_)
		return;
	ASSERT(saved_ == false);

	parent_ = sprite.parent();
	position_.set(sprite.x, sprite.y);
	rotation_ = sprite.rotation;
	scaleFactor_ = sprite.scaleFactor;
	anchorPoint_ = sprite.anchorPoint;
	color_ = sprite.color;

	const nc::Vector2f absPosition = sprite.absPosition();
	sprite.setParent(nullptr);
	sprite.setAbsPosition(absPosition);
	sprite.rotation = 0.0f;
	sprite.scaleFactor.set(1.0f, 1.0f);
	sprite.anchorPoint.set(0.0f, 0.0f);
	sprite.color = nc::Colorf::White;

	saved_ = true;
}

void UserInterface::SpriteProperties::restore(Sprite &sprite)
{
	if (saved_ == false)
		return;
	ASSERT(saved_ == true);

	sprite.setParent(parent_);
	sprite.x = position_.x;
	sprite.y = position_.y;
	sprite.rotation = rotation_;
	sprite.scaleFactor = scaleFactor_;
	sprite.anchorPoint = anchorPoint_;
	sprite.color = color_;

	saved_ = false;
}

void UserInterface::createCanvasWindow()
{
	ImGui::SetNextWindowSize(ImVec2(canvas_.texWidth() * canvasZoom_, canvas_.texHeight() * canvasZoom_), ImGuiCond_Once);
	ImGui::Begin("Canvas", nullptr, ImGuiWindowFlags_HorizontalScrollbar);
	const ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();
	ImGui::Image(canvas_.imguiTexId(), ImVec2(canvas_.texWidth() * canvasZoom_, canvas_.texHeight() * canvasZoom_), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));

	hoveringOnCanvas = false;
	if (ImGui::IsItemHovered() && spriteMgr_.sprites().isEmpty() == false)
	{
		hoveringOnCanvas = true;
		const ImVec2 mousePos = ImGui::GetMousePos();
		const ImVec2 relativePos(mousePos.x - cursorScreenPos.x, mousePos.y - cursorScreenPos.y);
		Sprite &sprite = *spriteMgr_.sprites()[selectedSpriteIndex_];
		const nc::Vector2f newSpriteAbsPos(roundf(relativePos.x / canvasZoom_), roundf(relativePos.y / canvasZoom_));
		const nc::Vector2f spriteRelativePos(newSpriteAbsPos - sprite.absPosition());

		static bool shiftAndClick = false;
		static bool ctrlAndClick = false;
		if (ImGui::GetIO().KeyShift || ImGui::GetIO().KeyCtrl)
		{
			if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
			{
				shiftAndClick = ImGui::GetIO().KeyShift;
				ctrlAndClick = ImGui::GetIO().KeyCtrl;

				// One frame lasting message
				statusMessage_.format("Coordinates: %d, %d", static_cast<int>(spriteRelativePos.x), static_cast<int>(spriteRelativePos.y));

				ImU32 color = IM_COL32(0, 0, 0, 255); // opaque black
				if (ImGui::GetIO().KeyShift)
					color |= 0x000000FF; // red
				if (ImGui::GetIO().KeyCtrl)
					color |= 0x00FF0000; // blue

				ImDrawList *drawList = ImGui::GetWindowDrawList();

				const ImRect spriteRect(cursorScreenPos.x + (sprite.absPosition().x - sprite.absWidth() / 2) * canvasZoom_,
				                        cursorScreenPos.y + (sprite.absPosition().y - sprite.absHeight() / 2) * canvasZoom_,
				                        cursorScreenPos.x + (sprite.absPosition().x + sprite.absWidth() / 2) * canvasZoom_,
				                        cursorScreenPos.y + (sprite.absPosition().y + sprite.absHeight() / 2) * canvasZoom_);
				drawList->AddRect(spriteRect.Min, spriteRect.Max, color, 0.0f, ImDrawCornerFlags_All, canvasZoom_);
				if (spriteRect.Contains(mousePos))
				{
					drawList->AddLine(ImVec2(spriteRect.Min.x, mousePos.y), ImVec2(spriteRect.Max.x, mousePos.y), color, canvasZoom_);
					drawList->AddLine(ImVec2(mousePos.x, spriteRect.Min.y), ImVec2(mousePos.x, spriteRect.Max.y), color, canvasZoom_);
				}

				spriteProps_.save(sprite);

				const float rectHalfSize = 2.0f * canvasZoom_;
				drawList->AddRectFilled(ImVec2(mousePos.x - rectHalfSize, mousePos.y - rectHalfSize), ImVec2(mousePos.x + rectHalfSize, mousePos.y + rectHalfSize), color);
				if (ImGui::GetIO().KeyCtrl &&
				    (spriteRelativePos.x != sprite.gridAnchorPoint.x || spriteRelativePos.y != sprite.gridAnchorPoint.y))
				{
					// Update grid anchor point while pressing Ctrl, clicking and moving the mouse
					sprite.gridAnchorPoint = spriteRelativePos;
					animMgr_.assignGridAnchorToParameters(&sprite);
				}
			}
			if (ImGui::GetIO().KeyShift && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			{
				spriteProps_.restore(sprite);

				// Update sprite anchor point while pressing Shift and releasing the mouse button
				sprite.anchorPoint = spriteRelativePos;
				sprite.setAbsPosition(newSpriteAbsPos);
			}
		}
		else
		{
			if (shiftAndClick || ctrlAndClick)
			{
				spriteProps_.restore(sprite);

				if (shiftAndClick)
				{
					// Update sprite anchor point while clicking the mouse button and releasing the Shift key
					sprite.anchorPoint = spriteRelativePos;
					sprite.setAbsPosition(newSpriteAbsPos);
				}
			}
			else if (ImGui::IsMouseDragging(ImGuiMouseButton_Left, 1.0f) || ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			{
				// Update sprite position while clicking and moving the mouse (with pixel snapping)
				if (sprite.absPosition().x != newSpriteAbsPos.x || sprite.absPosition().y != newSpriteAbsPos.y)
					sprite.setAbsPosition(newSpriteAbsPos);
			}

			shiftAndClick = false;
			ctrlAndClick = false;
		}
	}

	mouseWheelCanvasZoom();

	ImGui::End();
}

void UserInterface::createTexRectWindow()
{
	static MouseStatus mouseStatus_ = MouseStatus::IDLE;
	static ImVec2 startPos(0.0f, 0.0f);
	static ImVec2 endPos(0.0f, 0.0f);

	Sprite &sprite = *spriteMgr_.sprites()[selectedSpriteIndex_];
	nc::Recti texRect = sprite.texRect();

	ImVec2 size = ImVec2(sprite.texture().width() * canvasZoom_, sprite.texture().height() * canvasZoom_);
	ImGui::SetNextWindowSize(size, ImGuiCond_Once);
	ImGui::Begin("TexRect", &showTexrectWindow, ImGuiWindowFlags_HorizontalScrollbar);
	const ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();
	ImGui::Image(sprite.imguiTexId(), size);

	mouseWheelCanvasZoom();

	if (ImGui::IsItemHovered())
	{
		const ImVec2 mousePos = ImGui::GetMousePos();
		ImVec2 relPos(mousePos.x - cursorScreenPos.x, mousePos.y - cursorScreenPos.y);

		relPos.x = roundf(relPos.x / canvasZoom_);
		relPos.y = roundf(relPos.y / canvasZoom_);

		// One frame lasting message
		statusMessage_.format("Coordinates: %d, %d", static_cast<int>(relPos.x), static_cast<int>(relPos.y));
	}

	if (ImGui::IsWindowHovered())
	{
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			mouseStatus_ = MouseStatus::CLICKED;
			startPos = ImGui::GetMousePos();
		}
		else if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			mouseStatus_ = MouseStatus::DRAGGING;
			endPos = ImGui::GetMousePos();
		}
		else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			mouseStatus_ = MouseStatus::RELEASED;
			endPos = ImGui::GetMousePos();
		}
	}
	else if (mouseStatus_ == MouseStatus::CLICKED || mouseStatus_ == MouseStatus::DRAGGING)
		mouseStatus_ = MouseStatus::RELEASED;

	if (mouseStatus_ == MouseStatus::CLICKED)
	{
		// Clipping to image
		if (startPos.x > cursorScreenPos.x + size.x)
			startPos.x = cursorScreenPos.x + size.x;
		if (startPos.y > cursorScreenPos.y + size.y)
			startPos.y = cursorScreenPos.y + size.y;

		// Zoomed pixel snapping
		if (canvasZoom_ > 1.0f)
		{
			startPos.x = roundf((startPos.x - cursorScreenPos.x) / canvasZoom_) * canvasZoom_ + cursorScreenPos.x;
			startPos.y = roundf((startPos.y - cursorScreenPos.y) / canvasZoom_) * canvasZoom_ + cursorScreenPos.y;
		}
	}
	else if (mouseStatus_ == MouseStatus::DRAGGING || mouseStatus_ == MouseStatus::RELEASED)
	{
		// Clipping to image
		if (endPos.x < cursorScreenPos.x)
			endPos.x = cursorScreenPos.x;
		if (endPos.y < cursorScreenPos.y)
			endPos.y = cursorScreenPos.y;

		if (endPos.x > cursorScreenPos.x + size.x)
			endPos.x = cursorScreenPos.x + size.x;
		if (endPos.y > cursorScreenPos.y + size.y)
			endPos.y = cursorScreenPos.y + size.y;

		// Zoomed pixel snapping
		if (canvasZoom_ > 1.0f)
		{
			endPos.x = roundf((endPos.x - cursorScreenPos.x) / canvasZoom_) * canvasZoom_ + cursorScreenPos.x;
			endPos.y = roundf((endPos.y - cursorScreenPos.y) / canvasZoom_) * canvasZoom_ + cursorScreenPos.y;
		}
	}

	ImVec2 minRect(startPos);
	ImVec2 maxRect(endPos);

	if (mouseStatus_ == MouseStatus::IDLE || mouseStatus_ == MouseStatus::CLICKED ||
	    (!(maxRect.x - minRect.x != 0.0f && maxRect.y - minRect.y != 0.0f) && mouseStatus_ != MouseStatus::DRAGGING))
	{
		// Setting the non covered rect from the sprite texrect
		minRect.x = cursorScreenPos.x + (texRect.x * canvasZoom_);
		minRect.y = cursorScreenPos.y + (texRect.y * canvasZoom_);
		maxRect.x = minRect.x + (texRect.w * canvasZoom_);
		maxRect.y = minRect.y + (texRect.h * canvasZoom_);
	}
	else
	{
		// Setting the non covered rect from the mouse dragged area
		if (minRect.x > maxRect.x)
			nctl::swap(minRect.x, maxRect.x);
		if (minRect.y > maxRect.y)
			nctl::swap(minRect.y, maxRect.y);
	}

	const ImU32 darkGray = IM_COL32(0, 0, 0, 85);
	ImRect top(ImVec2(cursorScreenPos.x, cursorScreenPos.y), ImVec2(cursorScreenPos.x + size.x, minRect.y));
	ImRect left(ImVec2(cursorScreenPos.x, minRect.y), ImVec2(minRect.x, cursorScreenPos.y + size.y));
	ImRect right(ImVec2(maxRect.x, minRect.y), ImVec2(cursorScreenPos.x + size.x, cursorScreenPos.y + size.y));
	ImRect bottom(ImVec2(minRect.x, maxRect.y), ImVec2(maxRect.x, cursorScreenPos.y + size.y));
	ImDrawList *drawList = ImGui::GetWindowDrawList();
	drawList->AddRectFilled(top.Min, top.Max, darkGray);
	drawList->AddRectFilled(left.Min, left.Max, darkGray);
	drawList->AddRectFilled(right.Min, right.Max, darkGray);
	drawList->AddRectFilled(bottom.Min, bottom.Max, darkGray);

	if (mouseStatus_ == MouseStatus::RELEASED)
	{
		texRect.x = (minRect.x - cursorScreenPos.x) / canvasZoom_;
		texRect.y = (minRect.y - cursorScreenPos.y) / canvasZoom_;
		texRect.w = (maxRect.x - minRect.x) / canvasZoom_;
		texRect.h = (maxRect.y - minRect.y) / canvasZoom_;
		ASSERT(texRect.x >= 0);
		ASSERT(texRect.y >= 0);

		if (texRect.w > 0 && texRect.h > 0)
			sprite.setTexRect(texRect);

		// Back to idle mouse status after assigning the new texrect
		mouseStatus_ = MouseStatus::IDLE;
	}

	ImGui::End();
}

void UserInterface::createAboutWindow()
{
	ImGui::Begin("About", &showAboutWindow, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking);
	ImVec2 cursorPos = ImGui::GetCursorPos();
	const ImVec2 spookySize(spookyLogo_->width() * 2.0f, spookyLogo_->height() * 2.0f);
	cursorPos.x = (ImGui::GetWindowSize().x - spookySize.x) * 0.5f;
	ImGui::SetCursorPos(cursorPos);
	ImGui::Image(spookyLogo_->imguiTexId(), spookySize);
	ImGui::Spacing();
#ifdef WITH_GIT_VERSION
	ImGui::Text("SpookyGhost %s (%s)", VersionStrings::Version, VersionStrings::GitBranch);
#endif
	ImGui::Text("SpookyGhost compiled on %s at %s", __DATE__, __TIME__);
	ImGui::Spacing();
	ImGui::Text("https://encelo.itch.io/spookyghost");
	for (unsigned int i = 0; i < 4; i++)
		ImGui::Spacing();

	ImGui::Separator();

	for (unsigned int i = 0; i < 4; i++)
		ImGui::Spacing();
	cursorPos = ImGui::GetCursorPos();
	const ImVec2 ncineSize(ncineLogo_->width() * 2.0f, ncineLogo_->height() * 2.0f);
	cursorPos.x = (ImGui::GetWindowSize().x - ncineSize.x) * 0.5f;
	ImGui::SetCursorPos(cursorPos);
	ImGui::Image(ncineLogo_->imguiTexId(), spookySize);
	ImGui::Spacing();
	ImGui::Text("Based on nCine %s (%s)", nc::VersionStrings::Version, nc::VersionStrings::GitBranch);
	ImGui::Text("nCine compiled on %s at %s", nc::VersionStrings::CompilationDate, nc::VersionStrings::CompilationTime);
	ImGui::Spacing();
	ImGui::Text("https://ncine.github.io/");
	ImGui::End();
}

void UserInterface::mouseWheelCanvasZoom()
{
	if (ImGui::IsItemHovered() && ImGui::GetIO().KeyCtrl && ImGui::GetIO().MouseWheel != 0.0f)
	{
		const float wheel = ImGui::GetIO().MouseWheel;

		if (wheel > 0.0f)
			canvasZoom_ *= 2.0f;
		else if (wheel < 0.0f)
			canvasZoom_ *= 0.5f;

		if (canvasZoom_ > 8.0f)
			canvasZoom_ = 8.0f;
		else if (canvasZoom_ < 0.125f)
			canvasZoom_ = 0.125;
	}
}

void UserInterface::visitSprite(Sprite &sprite)
{
	sprite.visited = true;
	for (unsigned int i = 0; i < sprite.children().size(); i++)
		visitSprite(*sprite.children()[i]);
}

void UserInterface::removeSelectedSprite()
{
	if (spriteMgr_.sprites().isEmpty() == false)
	{
		animMgr_.removeSprite(spriteMgr_.sprites()[selectedSpriteIndex_].get());
		if (selectedSpriteIndex_ >= 0 && selectedSpriteIndex_ < spriteMgr_.sprites().size())
			spriteMgr_.sprites().removeAt(selectedSpriteIndex_);
		if (selectedSpriteIndex_ > 0)
			selectedSpriteIndex_--;
	}
}
