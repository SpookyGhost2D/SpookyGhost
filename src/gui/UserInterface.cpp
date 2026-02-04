#include <ctime>
#include "gui/gui_common.h"
#include <ncine/imgui_internal.h>
#include <ncine/InputEvents.h>
#include <ncine/Application.h>
#include <ncine/FileSystem.h>
#include <ncine/IFile.h>
#include <ncine/LuaStateManager.h>
#include <ncine/Random.h>

#include "singletons.h"
#include "gui/gui_labels.h"
#include "gui/gui_tips.h"
#include "gui/UserInterface.h"
#include "gui/FileDialog.h"
#include "Canvas.h"
#include "SpriteManager.h"
#include "AnimationManager.h"
#include "SpriteAnimation.h"
#include "Script.h"
#include "Sprite.h"
#include "Texture.h"
#include "ScriptManager.h"

#include "version.h"
#include <ncine/version.h>
#include "projects_strings.h"

namespace {

#if defined(__linux__) && !defined(__ANDROID__) && defined(NCPROJECT_DATA_DIR_DIST)
const char *docsFile = "../../doc/spookyghost/documentation.html";
#else
const char *docsFile = "../docs/documentation.html";
#endif

bool showTipsWindow = false;
bool showAboutWindow = false;
bool showQuitPopup = false;
bool showVideoModePopup = false;

unsigned int currentTipIndex = 0;

const float VideoModePopupTimeout = 15.0f;
nc::TimeStamp videoModeTimeStamp;
bool cancelVideoModeChange = false;

}

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

UserInterface::UserInterface()
    : selectedSpriteEntry_(&theSpriteMgr->root()),
      selectedTextureIndex_(0), selectedScriptIndex_(0),
      selectedAnimation_(&theAnimMgr->animGroup()),
      texturesWindow_(*this), spritesWindow_(*this), scriptsWindow_(*this), animationsWindow_(*this),
      spriteWindow_(*this), animationWindow_(*this), renderWindow_(*this), canvasWindows_(*this),
      saverData_(*theCanvas, *theSpriteMgr, *theScriptingMgr, *theAnimMgr)
{
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui::GetStyle().FontScaleMain = theCfg.guiScaling;

#ifdef WITH_FONTAWESOME
	io.Fonts->AddFontDefault(); // Cannot use `MergeMode` for the first font
	// Merge icons from Font Awesome into the default font
	static const ImWchar iconsRanges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	ImFontConfig iconsConfig;
	iconsConfig.MergeMode = true;
	iconsConfig.PixelSnapH = true;
	iconsConfig.FontDataOwnedByAtlas = false; // ImGui will otherwise try to use `free()` instead of `delete[]`

	// Loading font from memory so that a font can be an Android asset file
	nctl::UniquePtr<nc::IFile> fontFile = nc::IFile::createFileHandle(nc::fs::joinPath(nc::fs::dataPath(), "fonts/" FONT_ICON_FILE_NAME_FAS).data());
	fontFile->open(nc::IFile::OpenMode::READ);
	const long int fontFileSize = fontFile->size();
	fontFileBuffer_ = nctl::makeUnique<uint8_t[]>(fontFileSize);
	fontFile->read(fontFileBuffer_.get(), fontFileSize);
	io.Fonts->AddFontFromMemoryTTF(fontFileBuffer_.get(), fontFileSize, 12.0f, &iconsConfig, iconsRanges);
	fontFile->close();
#endif

	applyDarkStyle();

	spookyLogo_ = nctl::makeUnique<Texture>(nc::fs::joinPath(nc::fs::dataPath(), "icon96.png").data());
	ncineLogo_ = nctl::makeUnique<Texture>(nc::fs::joinPath(nc::fs::dataPath(), "ncine96.png").data());

	canvasGuiSection_.setResize(theCanvas->size());

	if (theCfg.startupProjectName.isEmpty() == false)
	{
		const nctl::String startupProject = nc::fs::joinPath(theCfg.projectsPath, theCfg.startupProjectName);
		if (nc::fs::isReadableFile(startupProject.data()))
			openProject(startupProject.data());
	}
	if (theCfg.autoPlayOnStart)
		theAnimMgr->play();

	Tips::initStrings();
	if (theCfg.showTipsOnStart)
		showTipsWindow = true;
	currentTipIndex = nc::random().fastInteger(0, Tips::Count);

	// The list of pinned directories in the file dialog will be saved in the configuration
	FileDialog::config.pinnedDirectories = &theCfg.pinnedDirectories;
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

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

const SaveAnim &UserInterface::saveAnimStatus() const
{
	return renderWindow_.saveAnimStatus();
}

bool UserInterface::shouldSaveFrames() const
{
	return renderWindow_.shouldSaveFrames();
}

bool UserInterface::shouldSaveSpritesheet() const
{
	return renderWindow_.shouldSaveSpritesheet();
}

void UserInterface::signalFrameSaved()
{
	renderWindow_.signalFrameSaved();
}

void UserInterface::cancelRender()
{
	renderWindow_.cancelRender();
}

void UserInterface::changeScalingFactor(float factor)
{
	ImGuiStyle &style = ImGui::GetStyle();

	if (style.FontScaleMain != factor)
	{
		style = ImGuiStyle();
		style.FontScaleMain = factor;
		applyDarkStyle();
		style.ScaleAllSizes(factor);
	}
}

void UserInterface::openVideoModePopup()
{
	videoModeTimeStamp = nc::TimeStamp::now();
	showVideoModePopup = true;
}

void UserInterface::closeModalsAndUndockables()
{
	showTipsWindow = false;
	showAboutWindow = false;
	showQuitPopup = false;
	// Don't close the popup but revert video mode
	cancelVideoModeChange = true;
	FileDialog::config.windowOpen = false;
}

void UserInterface::pressDeleteKey()
{
	deleteKeyPressed_ = true;
}

void UserInterface::moveSprite(int xDiff, int yDiff)
{
	if (selectedSpriteEntry_->isSprite() && canvasWindows_.isHoveringOnCanvas())
	{
		Sprite *sprite = selectedSpriteEntry_->toSprite();
		sprite->x += xDiff;
		sprite->y += yDiff;
	}
}

bool UserInterface::menuNewEnabled()
{
	return (theAnimMgr->anims().isEmpty() == false ||
	        theScriptingMgr->scripts().isEmpty() == false ||
	        theSpriteMgr->children().isEmpty() == false ||
	        theSpriteMgr->textures().isEmpty() == false);
}

bool UserInterface::menuSaveEnabled()
{
	return (lastLoadedProject_.isEmpty() == false &&
	        nc::fs::isReadableFile(lastLoadedProject_.data()) &&
	        menuNewEnabled());
}

bool UserInterface::menuSaveAsEnabled()
{
	return menuNewEnabled();
}

bool UserInterface::menuQuickOpenEnabled()
{
	return (lastQuickSavedProject_.isEmpty() == false &&
	        nc::fs::isReadableFile(lastQuickSavedProject_.data()));
}

bool UserInterface::menuQuickSaveEnabled()
{
	return menuNewEnabled();
}

void UserInterface::menuNew()
{
	selectedSpriteEntry_ = &theSpriteMgr->root();
	selectedAnimation_ = nullptr;
	// Always clear animations before sprites
	theAnimMgr->clear();
	theScriptingMgr->clear();
	theSpriteMgr->clear();
}

void UserInterface::menuOpen()
{
	FileDialog::config.directory = theCfg.projectsPath;
	FileDialog::config.windowIcon = Labels::FileDialog_OpenIcon;
	FileDialog::config.windowTitle = "Open project file";
	FileDialog::config.okButton = Labels::Ok;
	FileDialog::config.selectionType = FileDialog::SelectionType::FILE;
	FileDialog::config.extensions = "lua\0\0";
	FileDialog::config.action = FileDialog::Action::OPEN_PROJECT;
	FileDialog::config.windowOpen = true;
}

void UserInterface::menuSave()
{
	theSaver->save(lastLoadedProject_.data(), saverData_);
	ui::auxString.format("Saved project file \"%s\"\n", lastLoadedProject_.data());
	pushStatusInfoMessage(ui::auxString.data());
}

void UserInterface::menuSaveAs()
{
	FileDialog::config.directory = theCfg.projectsPath;
	FileDialog::config.windowIcon = Labels::FileDialog_SaveIcon;
	FileDialog::config.windowTitle = "Save project file";
	FileDialog::config.okButton = Labels::Ok;
	FileDialog::config.selectionType = FileDialog::SelectionType::NEW_FILE;
	FileDialog::config.extensions = nullptr;
	FileDialog::config.action = FileDialog::Action::SAVE_PROJECT;
	FileDialog::config.windowOpen = true;
}

void UserInterface::menuQuickOpen()
{
	if (openProject(lastQuickSavedProject_.data()))
		numFrames_ = 0; // force focus on the canvas
}

void UserInterface::menuQuickSave()
{
	time_t now;
	struct tm *ts;
	now = time(nullptr);
	ts = localtime(&now);

	lastQuickSavedProject_ = theCfg.projectsPath;
	nctl::String fileName(ui::MaxStringLength);

	const unsigned int length = strftime(fileName.data(), fileName.capacity() - 1, "quicksave_%Y%m%d_%H%M%S.lua", ts);
	fileName.setLength(length);
	lastQuickSavedProject_ = nc::fs::joinPath(lastQuickSavedProject_, fileName);

	theSaver->save(lastQuickSavedProject_.data(), saverData_);
	ui::auxString.format("Saved project file \"%s\"\n", lastQuickSavedProject_.data());
	pushStatusInfoMessage(ui::auxString.data());
}

void UserInterface::quit()
{
#ifdef __EMSCRIPTEN__
	nc::theApplication().quit();
#else
	if (FileDialog::config.modalPopup)
		FileDialog::config.windowOpen = false;
	showQuitPopup = true;
#endif
}

bool UserInterface::openDocumentationEnabled()
{
#ifdef __ANDROID__
	return false; // TODO: open with a WebView
#else
	nctl::String docsPath = nc::fs::joinPath(nc::fs::dataPath(), docsFile);
	return nc::fs::isReadableFile(docsPath.data());
#endif
}

void UserInterface::openDocumentation()
{
	nctl::String docsPath = nc::fs::joinPath(nc::fs::dataPath(), docsFile);
	// Can't use `Platform_OpenInShellUserData()` from ImGui
	openFile(docsPath.data());
}

void UserInterface::toggleAnimation()
{
	if (selectedAnimation_ && canvasWindows_.isHoveringOnCanvasWindow())
	{
		if (selectedAnimation_->state() != IAnimation::State::PLAYING)
			selectedAnimation_->play();
		else
			selectedAnimation_->pause();
	}
}

void UserInterface::reloadScript()
{
	scriptsWindow_.reloadScript();
}

void UserInterface::createGui()
{
	// Cache the initial value of the auto-suspension flag
	static bool autoSuspensionState = nc::theApplication().autoSuspension();

	if (lastStatus_.secondsSince() >= 2.0f)
		statusMessage_.clear();

	createDockingSpace();
	//createToolbarWindow();

	texturesWindow_.create();
	if (numFrames_ == 1)
		ImGui::SetNextWindowFocus();
	spritesWindow_.create();
	scriptsWindow_.create();
	animationsWindow_.create();

	if (numFrames_ == 1)
		ImGui::SetNextWindowFocus();
	spriteWindow_.create();
	animationWindow_.create();
	renderWindow_.create();

	createFileDialog();

	if (numFrames_ == 1)
		ImGui::SetNextWindowFocus();
	canvasWindows_.createCanvasWindow();
	canvasWindows_.createTexRectWindow();

	ImGui::Begin("Status");
	ImGui::Text("%s", statusMessage_.data());
	ImGui::End();

	if (showTipsWindow)
		createTipsWindow();

	if (showAboutWindow)
		createAboutWindow();

	if (showQuitPopup)
		createQuitPopup();

	if (showVideoModePopup)
		createVideoModePopup();

	createConfigWindow();

	deleteKeyPressed_ = false;
	if (enableKeyboardNav_)
	{
		// Enable keyboard navigation again
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	}
	enableKeyboardNav_ = true;
	if (numFrames_ < 2)
		numFrames_++;

	// Restore the initial auto-suspension state that might have been disabled by the drop callback
	if (ui::dropUpdateFrames > 0)
	{
		ui::dropUpdateFrames--;
		if (ui::dropUpdateFrames == 0)
			nc::theApplication().setAutoSuspension(autoSuspensionState);
	}
	// Reset the pointer if the event has not been handled elsewhere
	ui::dropEvent = nullptr;
}

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

bool UserInterface::openProject(const char *filename)
{
	if (nc::fs::isReadableFile(filename) && theSaver->load(filename, saverData_))
	{
		selectedSpriteEntry_ = &theSpriteMgr->root();
		selectedTextureIndex_ = 0;
		selectedScriptIndex_ = 0;
		selectedAnimation_ = &theAnimMgr->animGroup();

		canvasGuiSection_.setResize(theCanvas->size());
		renderWindow_.setResize(renderWindow_.saveAnimStatus().canvasResize);

		lastLoadedProject_ = filename;
		ui::auxString.format("Loaded project file \"%s\"\n", filename);
		pushStatusInfoMessage(ui::auxString.data());

		return true;
	}
	else
	{
		ui::auxString.format("Cannot load project file \"%s\"\n", filename);
		pushStatusErrorMessage(ui::auxString.data());

		return false;
	}
}

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
	const ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;
	const ImGuiID dockspaceId = ImGui::GetID("TheDockSpace");

	if (ImGui::DockBuilderGetNode(dockspaceId) != nullptr)
	{
		ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), dockspaceFlags);
		return;
	}

	ImGui::DockBuilderRemoveNode(dockspaceId);
	ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);

	ImGuiID dockMainId = dockspaceId;
	ImGuiID dockIdUp = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Up, 0.02f, nullptr, &dockMainId);
	ImGuiID dockIdDown = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Down, 0.02f, nullptr, &dockMainId);
	ImGuiID dockIdLeft = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Left, 0.275f, nullptr, &dockMainId);
	ImGuiID dockIdRight = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Right, 0.5f, nullptr, &dockMainId);
	ImGuiViewport *viewport = ImGui::GetMainViewport();
	ImGui::DockBuilderSetNodeSize(dockIdUp, ImVec2(viewport->Size.x, ImGui::GetFrameHeight()));
	ImGui::DockBuilderSetNodeSize(dockIdDown, ImVec2(viewport->Size.x, ImGui::GetFrameHeight()));
	ImGui::DockBuilderSetNodeSize(dockIdLeft, ImVec2(viewport->Size.x * 0.32f, viewport->Size.y));
	ImGui::DockBuilderSetNodeSize(dockIdRight, ImVec2(viewport->Size.x * 0.275f, viewport->Size.y));
	ImGuiID dockIdLeftDown = ImGui::DockBuilderSplitNode(dockIdLeft, ImGuiDir_Down, 0.35f, nullptr, &dockIdLeft);
	ImGuiID dockIdRightDown = ImGui::DockBuilderSplitNode(dockIdRight, ImGuiDir_Down, 0.335f, nullptr, &dockIdRight);

	ImGui::DockBuilderDockWindow("Toolbar", dockIdUp);

	ImGui::DockBuilderDockWindow(Labels::Textures, dockIdLeft);
	ImGui::DockBuilderDockWindow(Labels::Sprites, dockIdLeft);
	ImGui::DockBuilderDockWindow(Labels::Scripts, dockIdLeft);
	ImGui::DockBuilderDockWindow(Labels::Animations, dockIdLeftDown);

	ImGui::DockBuilderDockWindow(Labels::Canvas, dockMainId);
	ImGui::DockBuilderDockWindow(Labels::TexRect, dockMainId);

	ImGui::DockBuilderDockWindow(Labels::Sprite, dockIdRight);
	ImGui::DockBuilderDockWindow(Labels::Animation, dockIdRight);
	ImGui::DockBuilderDockWindow(Labels::Render, dockIdRightDown);

	ImGui::DockBuilderDockWindow("Status", dockIdDown);

	ImGuiDockNode *node = ImGui::DockBuilderGetNode(dockIdUp);
	node->LocalFlags |= (ImGuiDockNodeFlags_NoTabBar);

	node = ImGui::DockBuilderGetNode(dockIdDown);
	node->LocalFlags |= (ImGuiDockNodeFlags_NoTabBar);

	ImGui::DockBuilderFinish(dockspaceId);
}

void UserInterface::createMenuBar()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem(Labels::New, "CTRL + N", false, menuNewEnabled()))
				menuNew();

			if (ImGui::MenuItem(Labels::Open, "CTRL + O"))
				menuOpen();

			const bool openBundledEnabled = ProjectsStrings::Count > 0;
			if (ImGui::BeginMenu(Labels::OpenBundled, openBundledEnabled))
			{
				for (unsigned int i = 0; i < ProjectsStrings::Count; i++)
				{
					if (ImGui::MenuItem(ProjectsStrings::Names[i]))
					{
						if (openProject(nc::fs::joinPath(ui::projectsDataDir, ProjectsStrings::Names[i]).data()))
							numFrames_ = 0; // force focus on the canvas
					}
				}
				ImGui::EndMenu();
			}

			if (ImGui::MenuItem(Labels::Save, "CTRL + S", false, menuSaveEnabled()))
				menuSave();

			if (ImGui::MenuItem(Labels::SaveAs, nullptr, false, menuSaveAsEnabled()))
				menuSaveAs();

			ImGui::Separator();

			if (ImGui::MenuItem(Labels::QuickOpen, "F9", false, menuQuickOpenEnabled()))
				menuQuickOpen();

			if (ImGui::MenuItem(Labels::QuickSave, "F5", false, menuQuickSaveEnabled()))
				menuQuickSave();

			ImGui::Separator();

			if (ImGui::MenuItem(Labels::Configuration))
				showConfigWindow = true;

			ImGui::Separator();

			if (ImGui::MenuItem(Labels::Quit, "CTRL + Q"))
				quit();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help"))
		{
			if (ImGui::MenuItem(Labels::Documentation, "F1", false, openDocumentationEnabled()))
				openDocumentation();

			if (ImGui::MenuItem(Labels::Tips))
				showTipsWindow = true;

			if (ImGui::MenuItem(Labels::About))
				showAboutWindow = true;

			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
}

void UserInterface::createToolbarWindow()
{
	const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_None | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("Toolbar", nullptr, windowFlags);

	const ImVec2 closeItemSpacing(ImGui::GetStyle().ItemSpacing.x * 0.5f, ImGui::GetStyle().ItemSpacing.y * 0.5f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, closeItemSpacing);
	if (ImGui::Button(Labels::New) && menuNewEnabled())
		menuNew();
	ImGui::SameLine();
	if (ImGui::Button(Labels::Open))
		menuOpen();
	ImGui::SameLine();
	if (ImGui::Button(Labels::Save) && menuSaveEnabled())
		menuSave();
	ImGui::SameLine();
	if (ImGui::Button(Labels::SaveAs) && menuSaveAsEnabled())
		menuSaveAs();

	ImGui::SameLine();
	if (ImGui::Button(Labels::QuickOpen) && menuQuickOpenEnabled())
		menuQuickOpen();
	ImGui::SameLine();
	if (ImGui::Button(Labels::QuickSave) && menuQuickSaveEnabled())
		menuQuickSave();

	ImGui::SameLine();
	if (ImGui::Button(Labels::Quit))
		quit();
	ImGui::PopStyleVar(2);

	ImGui::End();
}

void UserInterface::createFileDialog()
{
	static nctl::String selection = nctl::String(nc::fs::MaxPathLength);

	if (FileDialog::create(FileDialog::config, selection))
	{
		switch (FileDialog::config.action)
		{
			case FileDialog::Action::OPEN_PROJECT:
				if (openProject(selection.data()))
					numFrames_ = 0; // force focus on the canvas
				break;
			case FileDialog::Action::SAVE_PROJECT:
				if (nc::fs::hasExtension(selection.data(), "lua") == false)
					selection = selection + ".lua";
				if (nc::fs::isFile(selection.data()) && FileDialog::config.allowOverwrite == false)
				{
					ui::auxString.format("Cannot overwrite existing file \"%s\"\n", selection.data());
					pushStatusErrorMessage(ui::auxString.data());
				}
				else
				{
#ifdef __EMSCRIPTEN__
					theSaver->save(nc::fs::baseName(selection.data()).data(), saverData_);
					ui::auxString.format("Saved project file \"%s\"\n", nc::fs::baseName(selection.data()).data());
#else
					theSaver->save(selection.data(), saverData_);
					ui::auxString.format("Saved project file \"%s\"\n", selection.data());
#endif
					pushStatusInfoMessage(ui::auxString.data());
				}
				break;
			case FileDialog::Action::LOAD_TEXTURE:
				texturesWindow_.loadTexture(selection.data());
				break;
			case FileDialog::Action::RELOAD_TEXTURE:
				texturesWindow_.reloadTexture(selection.data());
				break;
			case FileDialog::Action::LOAD_SCRIPT:
				scriptsWindow_.loadScript(selection.data());
				break;
			case FileDialog::Action::RENDER_DIR:
				renderWindow_.directory = selection;
				break;
		}
	}
}

void UserInterface::createTipsWindow()
{
	if (showTipsWindow == false)
		return;

	const ImVec2 windowPos = ImVec2(ImGui::GetWindowViewport()->Size.x * 0.5f, ImGui::GetWindowViewport()->Size.y * 0.5f);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));

	ImGui::Begin("Tips", &showTipsWindow, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking);
	ImGui::Text("%s Did you know...", Labels::LightbulbIcon);
	ImGui::NewLine();
	const float startY = ImGui::GetCursorPosY();
	ImGui::TextWrapped("%s", Tips::Strings[currentTipIndex].data());
	const float endY = ImGui::GetCursorPosY();
	const unsigned int numLines = (endY - startY) / ImGui::GetTextLineHeight();

	// Check if `MaxNumberLines` has not been updated to the longest tip string
	if (ImGui::IsWindowAppearing() == false)
		ASSERT(numLines <= Tips::MaxNumberLines);

	if (numLines < Tips::MaxNumberLines)
	{
		static char emptyLines[Tips::MaxNumberLines + 1] = "\0";
		for (unsigned int i = 0; i < Tips::MaxNumberLines - numLines; i++)
			emptyLines[i] = '\n';
		emptyLines[Tips::MaxNumberLines - numLines] = '\0';
		const ImVec2 closeItemSpacing(ImGui::GetStyle().ItemSpacing.x * 0.5f, ImGui::GetStyle().ItemSpacing.y * 0.5f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, closeItemSpacing);
		ImGui::TextUnformatted(emptyLines);
		ImGui::PopStyleVar();
	}

	ImGui::NewLine();
	ImGui::Checkbox("Show Tips On Start", &theCfg.showTipsOnStart);

	ImGui::SameLine();
	ui::auxString.format("%s Prev", Labels::PreviousIcon);
	const bool enablePrevButton = (currentTipIndex >= 1);
	ImGui::BeginDisabled(enablePrevButton == false);
	if (ImGui::Button(ui::auxString.data()))
		currentTipIndex--;
	ImGui::EndDisabled();

	ImGui::SameLine();
	ImGui::Text("%u / %u", currentTipIndex + 1, Tips::Count);

	ImGui::SameLine();
	ui::auxString.format("Next %s", Labels::NextIcon);
	const bool enableNextButton = (currentTipIndex <= Tips::Count - 2);
	ImGui::BeginDisabled(enableNextButton == false);
	if (ImGui::Button(ui::auxString.data()))
		currentTipIndex++;
	ImGui::EndDisabled();

	ImGui::SameLine();
	if (ImGui::Button(Labels::Close))
		showTipsWindow = false;

	// Auto-save on window close
	if (showTipsWindow == false)
		theSaver->saveCfg(theCfg);

	if (ImGui::IsWindowHovered())
	{
		// Disable keyboard navigation
		ImGui::GetIO().ConfigFlags &= ~(ImGuiConfigFlags_NavEnableKeyboard);
		enableKeyboardNav_ = false;

		if (enablePrevButton && ImGui::IsKeyReleased(ImGuiKey_LeftArrow))
			currentTipIndex--;
		if (enableNextButton && ImGui::IsKeyReleased(ImGuiKey_RightArrow))
			currentTipIndex++;
	}

	ImGui::End();
}

void UserInterface::createAboutWindow()
{
	const ImVec2 windowPos = ImVec2(ImGui::GetWindowViewport()->Size.x * 0.5f, ImGui::GetWindowViewport()->Size.y * 0.5f);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));

	ImGui::Begin("About", &showAboutWindow, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking);
	ImVec2 cursorPos = ImGui::GetCursorPos();
	const ImVec2 spookySize(spookyLogo_->width() * 2.0f, spookyLogo_->height() * 2.0f);
	cursorPos.x = (ImGui::GetWindowSize().x - spookySize.x) * 0.5f;
	ImGui::SetCursorPos(cursorPos);
	ImGui::Image(static_cast<ImTextureID>(reinterpret_cast<intptr_t>(spookyLogo_->imguiTexId())), spookySize);
	ImGui::Spacing();
#ifdef WITH_GIT_VERSION
	ImGui::Text("SpookyGhost %s (%s)", VersionStrings::Version, VersionStrings::GitBranch);
#endif
	ImGui::Text("Compiled on %s at %s", __DATE__, __TIME__);
	ImGui::Spacing();
	ImGui::TextLinkOpenURL("https://encelo.itch.io/spookyghost", "https://encelo.itch.io/spookyghost");
	for (unsigned int i = 0; i < 4; i++)
		ImGui::Spacing();

	ImGui::Separator();

	for (unsigned int i = 0; i < 4; i++)
		ImGui::Spacing();
	cursorPos = ImGui::GetCursorPos();
	const ImVec2 ncineSize(ncineLogo_->width() * 2.0f, ncineLogo_->height() * 2.0f);
	cursorPos.x = (ImGui::GetWindowSize().x - ncineSize.x) * 0.5f;
	ImGui::SetCursorPos(cursorPos);
	ImGui::Image(static_cast<ImTextureID>(reinterpret_cast<intptr_t>(ncineLogo_->imguiTexId())), spookySize);
	ImGui::Spacing();
	ImGui::Text("nCine %s (%s)", nc::VersionStrings::Version, nc::VersionStrings::GitBranch);
	ImGui::Text("Compiled on %s at %s", nc::VersionStrings::CompilationDate, nc::VersionStrings::CompilationTime);
	ImGui::Spacing();
	ImGui::TextLinkOpenURL("https://ncine.github.io/", "https://ncine.github.io/");

	ImGui::NewLine();
	if (ImGui::Button(Labels::Close))
		showAboutWindow = false;

	ImGui::End();
}

void UserInterface::createQuitPopup()
{
	const ImVec2 windowPos = ImVec2(ImGui::GetWindowViewport()->Size.x * 0.5f, ImGui::GetWindowViewport()->Size.y * 0.5f);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	ImGui::OpenPopup(Labels::Quit);
	ImGui::BeginPopupModal(Labels::Quit, nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::TextUnformatted("Quit the program?");
	if (ImGui::Button(Labels::Ok))
	{
		// Save the configuration on quit
		// It is the only chance to save the list of pinned directories and update window position and size
		nc::IGfxDevice &gfxDevice = nc::theApplication().gfxDevice();
		theCfg.windowPositionX = gfxDevice.windowPositionX();
		theCfg.windowPositionY = gfxDevice.windowPositionY();
		if (theCfg.fullScreen == false)
		{
			const float scalingFactor = theCfg.autoGuiScaling ? gfxDevice.windowScalingFactor() : 1.0f;
			theCfg.width = gfxDevice.width() / scalingFactor;
			theCfg.height = gfxDevice.height() / scalingFactor;
		}
		theSaver->saveCfg(theCfg);

		nc::theApplication().quit();
		showQuitPopup = false;
		ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button(Labels::Cancel))
	{
		showQuitPopup = false;
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}

void UserInterface::createVideoModePopup()
{
	const ImVec2 windowPos = ImVec2(ImGui::GetWindowViewport()->Size.x * 0.5f, ImGui::GetWindowViewport()->Size.y * 0.5f);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	ImGui::OpenPopup(Labels::VideoModeChanged);
	ImGui::BeginPopupModal(Labels::VideoModeChanged, nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	const int remainingSeconds = static_cast<int>(VideoModePopupTimeout - videoModeTimeStamp.secondsSince());

	ImGui::Text("Keep the current video mode? (%d s)", remainingSeconds);
	if (ImGui::Button(Labels::Ok))
	{
		// Save the video mode configuration
		nc::IGfxDevice &gfxDevice = nc::theApplication().gfxDevice();
		const nc::IGfxDevice::VideoMode &videoMode = gfxDevice.currentVideoMode();
		theCfg.width = videoMode.width;
		theCfg.height = videoMode.height;
		theCfg.refreshRate = videoMode.refreshRate;
		theCfg.fullScreen = gfxDevice.isFullScreen();
		theCfg.windowPositionX = gfxDevice.windowPositionX();
		theCfg.windowPositionX = gfxDevice.windowPositionY();
		theSaver->saveCfg(theCfg);

		showVideoModePopup = false;
		ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button(Labels::Cancel) || remainingSeconds <= 0 || cancelVideoModeChange)
	{
		nc::IGfxDevice &gfxDevice = nc::theApplication().gfxDevice();
		const unsigned int monitorIndex = gfxDevice.windowMonitorIndex();
		const nc::IGfxDevice::Monitor &monitor = gfxDevice.monitor(monitorIndex);
		const unsigned int numVideoModes = monitor.numVideoModes;
		unsigned int previousModeIndex = 0;
		for (unsigned int modeIndex = 0; modeIndex < numVideoModes; modeIndex++)
		{
			const nc::IGfxDevice::VideoMode &mode = monitor.videoModes[modeIndex];
			if (mode.width == theCfg.width && mode.height == theCfg.height && mode.refreshRate == theCfg.refreshRate)
			{
				previousModeIndex = modeIndex;
				break;
			}
		}
		if (theCfg.fullScreen)
			gfxDevice.setVideoMode(previousModeIndex);
		gfxDevice.setFullScreen(theCfg.fullScreen);
		if (theCfg.fullScreen == false)
			gfxDevice.setWindowPosition(theCfg.windowPositionX, theCfg.windowPositionY);

		cancelVideoModeChange = false;
		showVideoModePopup = false;
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}

void UserInterface::updateSelectedAnimOnSpriteRemoval(Sprite *sprite)
{
	if (selectedAnimation_ && selectedAnimation_->isSprite())
	{
		SpriteAnimation &spriteAnim = static_cast<SpriteAnimation &>(*selectedAnimation_);
		if (spriteAnim.sprite() == sprite)
			selectedAnimation_ = nullptr;
	}
}
