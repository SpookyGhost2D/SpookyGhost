#include <ctime>
#include "gui/gui_common.h"
#include <ncine/imgui_internal.h>
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
#include "PropertyAnimation.h"
#include "ParallelAnimationGroup.h"
#include "SequentialAnimationGroup.h"
#include "GridAnimation.h"
#include "GridFunctionLibrary.h"
#include "Script.h"
#include "ScriptAnimation.h"
#include "Sprite.h"
#include "Texture.h"
#include "ScriptManager.h"

#include "version.h"
#include <ncine/version.h>
#include "projects_strings.h"
#if defined(__ANDROID__) || defined(__EMSCRIPTEN__)
	#include "textures_strings.h"
#endif
#include "scripts_strings.h"

namespace {

// clang-format off
const char *anchorPointItems[] = { "Center", "Bottom Left", "Top Left", "Bottom Right", "Top Right" };
enum AnchorPointsEnum { CENTER, BOTTOM_LEFT, TOP_LEFT, BOTTOM_RIGHT, TOP_RIGHT };
const char *blendingPresets[] = { "Disabled", "Alpha", "Pre-multiplied Alpha", "Additive", "Multiply" };

const char *easingCurveTypes[] = { "Linear", "Quadratic", "Cubic", "Quartic", "Quintic", "Sine", "Exponential", "Circular" };
const char *loopDirections[] = { "Forward", "Backward" };
const char *loopModes[] = { "Disabled", "Rewind", "Ping Pong" };

const char *animationTypes[] = { "Parallel Group", "Sequential Group", "Property", "Grid", "Script" };
enum AnimationTypesEnum { PARALLEL_GROUP, SEQUENTIAL_GROUP, PROPERTY, GRID, SCRIPT };
// clang-format on

static const int PlotArraySize = 512;
static float plotArray[PlotArraySize];
static int plotValueIndex = 0;

const char *docsFile = "../docs/documentation.html";

static bool showTipsWindow = false;
static bool showAboutWindow = false;
static bool showQuitPopup = false;
static bool hoveringOnCanvasWindow = false;
static bool hoveringOnCanvas = false;
static bool deleteKeyPressed = false;

static int numFrames = 0;
static unsigned int currentTipIndex = 0;

const float imageTooltipSize = 100.0f;
const float imageTooltipDelay = 0.5f;

}

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

UserInterface::UserInterface()
    : selectedSpriteIndex_(0), selectedTextureIndex_(0), selectedScriptIndex_(0),
      selectedAnimation_(&theAnimMgr->animGroup()), spriteGraph_(4), renderGuiWindow_(*this),
      saverData_(*theCanvas, *theSpriteMgr, *theScriptingMgr, *theAnimMgr)
{
#ifdef __EMSCRIPTEN__
	textureLocalFile_.setLoadedCallback([](const nc::EmscriptenLocalFile &localFile, void *userData) {
		UserInterface *ui = reinterpret_cast<UserInterface *>(userData);
		if (localFile.size() > 0)
			ui->loadTexture(localFile.filename(), localFile.data(), localFile.size());
	}, this);
#endif

	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	io.FontGlobalScale = theCfg.guiScaling;

#ifdef WITH_FONTAWESOME
	// Merge icons from Font Awesome into the default font
	static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	ImFontConfig icons_config;
	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;

	// Loading font from memory so that a font can be an Android asset file
	nctl::UniquePtr<nc::IFile> fontFile = nc::IFile::createFileHandle(nc::fs::joinPath(nc::fs::dataPath(), "fonts/" FONT_ICON_FILE_NAME_FAS).data());
	fontFile->open(nc::IFile::OpenMode::READ);
	const long int fontFileSize = fontFile->size();
	nctl::UniquePtr<uint8_t[]> fontFileBuffer = nctl::makeUnique<uint8_t[]>(fontFileSize);
	fontFile->read(fontFileBuffer.get(), fontFileSize);
	io.Fonts->AddFontFromMemoryTTF(fontFileBuffer.get(), fontFileSize, 12.0f, &icons_config, icons_ranges);
	// Transfer ownership to ImGui
	fontFileBuffer.release();
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

	if (theCfg.showTipsOnStart)
		showTipsWindow = true;
	currentTipIndex = nc::random().fastInteger(0, Tips::Count);
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
	return renderGuiWindow_.saveAnimStatus();
}

bool UserInterface::shouldSaveFrames() const
{
	return renderGuiWindow_.shouldSaveFrames();
}

bool UserInterface::shouldSaveSpritesheet() const
{
	return renderGuiWindow_.shouldSaveSpritesheet();
}

void UserInterface::signalFrameSaved()
{
	renderGuiWindow_.signalFrameSaved();
}

void UserInterface::cancelRender()
{
	renderGuiWindow_.cancelRender();
}

void UserInterface::closeModalsAndUndockables()
{
	showTipsWindow = false;
	showAboutWindow = false;
	showQuitPopup = false;
	FileDialog::config.windowOpen = false;
}

void UserInterface::pressDeleteKey()
{
	deleteKeyPressed = true;
}

void UserInterface::moveSprite(int xDiff, int yDiff)
{
	if (theSpriteMgr->sprites().isEmpty() == false && hoveringOnCanvas)
	{
		Sprite &sprite = *theSpriteMgr->sprites()[selectedSpriteIndex_];
		sprite.x += xDiff;
		sprite.y += yDiff;
	}
}

bool UserInterface::menuNewEnabled()
{
	return (theAnimMgr->anims().isEmpty() == false ||
	        theScriptingMgr->scripts().isEmpty() == false ||
	        theSpriteMgr->sprites().isEmpty() == false ||
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
	selectedAnimation_ = nullptr;
	// Always clear animations before sprites
	theAnimMgr->clear();
	theScriptingMgr->scripts().clear();
	theSpriteMgr->sprites().clear();
	theSpriteMgr->textures().clear();
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
		numFrames = 0; // force focus on the canvas
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
#if defined(__ANDROID__) || defined(__EMSCRIPTEN__)
	nc::theApplication().quit();
#else
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
	openFile(docsPath.data());
}

void UserInterface::toggleAnimation()
{
	if (selectedAnimation_ && hoveringOnCanvasWindow)
	{
		if (selectedAnimation_->state() != IAnimation::State::PLAYING)
			selectedAnimation_->play();
		else
			selectedAnimation_->pause();
	}
}

void UserInterface::reloadScript()
{
	if (theScriptingMgr->scripts().isEmpty() == false)
	{
		Script *script = theScriptingMgr->scripts()[selectedScriptIndex_].get();
		script->reload();
		theAnimMgr->reloadScript(script);

		ui::auxString.format("Reloaded script \"%s\"\n", script->name().data());
		pushStatusInfoMessage(ui::auxString.data());
	}
}

void UserInterface::createGui()
{
	if (lastStatus_.secondsSince() >= 2.0f)
		statusMessage_.clear();

	createDockingSpace();
	//createToolbarWindow();

	createTexturesWindow();
	if (numFrames == 1)
		ImGui::SetNextWindowFocus();
	createSpritesWindow();
	createScriptsWindow();
	createAnimationsWindow();

	if (numFrames == 1)
		ImGui::SetNextWindowFocus();
	createSpriteWindow();
	createAnimationWindow();
	renderGuiWindow_.create();

	createFileDialog();

	if (numFrames == 1)
		ImGui::SetNextWindowFocus();
	createCanvasWindow();
	if (theSpriteMgr->sprites().isEmpty() == false)
		createTexRectWindow();

	ImGui::Begin("Status");
	ImGui::Text("%s", statusMessage_.data());
	ImGui::End();

	if (showTipsWindow)
		createTipsWindow();

	if (showAboutWindow)
		createAboutWindow();

	if (showQuitPopup)
		createQuitPopup();

	createConfigWindow();

	deleteKeyPressed = false;
	if (numFrames < 2)
		numFrames++;
}

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

bool UserInterface::openProject(const char *filename)
{
	if (nc::fs::isReadableFile(filename) && theSaver->load(filename, saverData_))
	{
		selectedSpriteIndex_ = 0;
		selectedTextureIndex_ = 0;
		selectedScriptIndex_ = 0;
		selectedAnimation_ = &theAnimMgr->animGroup();

		canvasGuiSection_.setResize(theCanvas->size());
		renderGuiWindow_.setResize(renderGuiWindow_.saveAnimStatus().canvasResize);

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

bool UserInterface::loadTexture(const char *filename)
{
	nctl::UniquePtr<Texture> texture = nctl::makeUnique<Texture>(filename);
	return loadTextureImpl(nctl::move(texture), filename);
}

bool UserInterface::loadScript(const char *filename)
{
	nctl::UniquePtr<Script> script = nctl::makeUnique<Script>();
	const bool hasLoaded = script->load(filename);

	theScriptingMgr->scripts().pushBack(nctl::move(script));
	// Check if the script is in the configuration path or in the data directory
	const nctl::String baseName = nc::fs::baseName(filename);
	const nctl::String nameInConfigDir = nc::fs::joinPath(theCfg.scriptsPath, baseName);
	const nctl::String nameInDataDir = nc::fs::joinPath(ui::scriptsDataDir, baseName);
	if ((nameInConfigDir == filename && nc::fs::isReadableFile(nameInConfigDir.data())) ||
	    (nameInDataDir == filename && nc::fs::isReadableFile(nameInDataDir.data())))
	{
		// Set the script name to its basename to allow for relocatable project files
		theScriptingMgr->scripts().back()->setName(baseName);
	}
	selectedScriptIndex_ = theScriptingMgr->scripts().size() - 1;

	if (hasLoaded)
	{
		if (theScriptingMgr->scripts().back()->canRun())
		{
			ui::auxString.format("Loaded script \"%s\"", filename);
			pushStatusInfoMessage(ui::auxString.data());
		}
		else
		{
			ui::auxString.format("Loaded script \"%s\", but it cannot run", filename);
			pushStatusErrorMessage(ui::auxString.data());
		}
	}
	else
	{
		ui::auxString.format("Cannot load script \"%s\"", filename);
		pushStatusErrorMessage(ui::auxString.data());
	}

	return hasLoaded;
}

#ifdef __EMSCRIPTEN__
bool UserInterface::loadTexture(const char *bufferName, const char *bufferPtr, unsigned long int bufferSize)
{
	nctl::UniquePtr<Texture> texture = nctl::makeUnique<Texture>(bufferName, reinterpret_cast<const unsigned char *>(bufferPtr), bufferSize);
	return loadTextureImpl(nctl::move(texture), bufferName);
}
#endif

bool UserInterface::loadTextureImpl(nctl::UniquePtr<Texture> texture, const char *name)
{
	if (texture->dataSize() > 0)
	{
		theSpriteMgr->textures().pushBack(nctl::move(texture));
		// Check if the texture is in the configuration path or in the data directory
		const nctl::String baseName = nc::fs::baseName(name);
		const nctl::String nameInConfigDir = nc::fs::joinPath(theCfg.texturesPath, baseName);
		const nctl::String nameInDataDir = nc::fs::joinPath(ui::texturesDataDir, baseName);
		if ((nameInConfigDir == name && nc::fs::isReadableFile(nameInConfigDir.data())) ||
		    (nameInDataDir == name && nc::fs::isReadableFile(nameInDataDir.data())))
		{
			// Set the texture name to its basename to allow for relocatable project files
			theSpriteMgr->textures().back()->setName(baseName);
		}
		selectedTextureIndex_ = theSpriteMgr->textures().size() - 1;

		ui::auxString.format("Loaded texture \"%s\"", name);
		pushStatusInfoMessage(ui::auxString.data());

		return true;
	}
	else
	{
		ui::auxString.format("Cannot load texture \"%s\"", name);
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
	ImGui::DockBuilderSetNodeSize(dockIdRight, ImVec2(viewport->Size.x * 0.275f, viewport->Size.y));
	ImGuiID dockIdLeftDown = ImGui::DockBuilderSplitNode(dockIdLeft, ImGuiDir_Down, 0.35f, nullptr, &dockIdLeft);
	ImGuiID dockIdRightDown = ImGui::DockBuilderSplitNode(dockIdRight, ImGuiDir_Down, 0.33f, nullptr, &dockIdRight);

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
							numFrames = 0; // force focus on the canvas
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

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
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
	ImGui::PopStyleVar(2);

	ImGui::End();
}

void UserInterface::removeTexture()
{
	// Deleting backwards without iterators
	for (int i = theSpriteMgr->sprites().size() - 1; i >= 0; i--)
	{
		Sprite *sprite = theSpriteMgr->sprites()[i].get();
		if (&sprite->texture() == theSpriteMgr->textures()[selectedTextureIndex_].get())
		{
			updateSelectedAnimOnSpriteRemoval(sprite);
			theAnimMgr->removeSprite(sprite);
			theSpriteMgr->sprites().removeAt(i);
			if (selectedSpriteIndex_ == i)
				selectedSpriteIndex_--;
		}
	}
	if (selectedSpriteIndex_ > theSpriteMgr->sprites().size() - 1)
		selectedSpriteIndex_ = theSpriteMgr->sprites().size() - 1;
	theSpriteMgr->textures().removeAt(selectedTextureIndex_);
	if (selectedTextureIndex_ > 0)
		selectedTextureIndex_--;
}

void UserInterface::createTexturesWindow()
{
	ImGui::Begin(Labels::Textures);

#if defined(__ANDROID__) || defined(__EMSCRIPTEN__)
	const bool openBundledEnabled = TexturesStrings::Count > 0;
	if (openBundledEnabled)
	{
		ui::comboString.clear();
		ui::comboString.append(Labels::BundledTextures);
		ui::comboString.setLength(ui::comboString.length() + 1);
		for (unsigned int i = 0; i < TexturesStrings::Count; i++)
		{
			ui::comboString.formatAppend("%s", TexturesStrings::Names[i]);
			ui::comboString.setLength(ui::comboString.length() + 1);
		}
		ui::comboString.setLength(ui::comboString.length() + 1);
		// Append a second '\0' to signal the end of the combo item list
		ui::comboString[ui::comboString.length() - 1] = '\0';

		static int currentComboTexture = 0;
		if (ImGui::Combo("###BundledTextures", &currentComboTexture, ui::comboString.data()) && currentComboTexture > 0)
		{
			loadTexture(nc::fs::joinPath(ui::texturesDataDir, TexturesStrings::Names[currentComboTexture - 1]).data());
			currentComboTexture = 0;
		}
	}
#endif

	if (ImGui::Button(Labels::Load))
	{
#ifndef __EMSCRIPTEN__
		FileDialog::config.directory = theCfg.texturesPath;
		FileDialog::config.windowIcon = Labels::FileDialog_OpenIcon;
		FileDialog::config.windowTitle = "Load texture file";
		FileDialog::config.okButton = Labels::Ok;
		FileDialog::config.selectionType = FileDialog::SelectionType::FILE;
		FileDialog::config.extensions = "png\0\0";
		FileDialog::config.action = FileDialog::Action::LOAD_TEXTURE;
		FileDialog::config.windowOpen = true;
#else
		textureLocalFile_.load();
#endif
	}

	if (theSpriteMgr->textures().isEmpty() == false)
	{
		ImGui::SameLine();
		if (ImGui::Button(Labels::Remove) || (deleteKeyPressed && ImGui::IsWindowHovered()))
			removeTexture();
	}

	if (theSpriteMgr->textures().isEmpty() == false)
	{
		ImGui::Separator();
		for (unsigned int i = 0; i < theSpriteMgr->textures().size(); i++)
		{
			Texture &texture = *theSpriteMgr->textures()[i];
			ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
			if (i == selectedTextureIndex_)
				nodeFlags |= ImGuiTreeNodeFlags_Selected;

			ImGui::TreeNodeEx(static_cast<void *>(&texture), nodeFlags, "#%u: \"%s\" (%d x %d)",
			                  i, texture.name().data(), texture.width(), texture.height());

			if (ImGui::IsItemHovered() && ImGui::GetCurrentContext()->HoveredIdTimer > imageTooltipDelay)
			{
				if (texture.width() > 0 && texture.height() > 0)
				{
					const float aspectRatio = texture.width() / static_cast<float>(texture.height());
					float width = imageTooltipSize;
					float height = imageTooltipSize;
					if (aspectRatio > 1.0f)
						height = width / aspectRatio;
					else
						width *= aspectRatio;

					ImGui::BeginTooltip();
					ImGui::Image(texture.imguiTexId(), ImVec2(width, height), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
					ImGui::EndTooltip();
				}
			}
			if (ImGui::IsItemClicked())
				selectedTextureIndex_ = i;

			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::MenuItem(Labels::Remove))
				{
					selectedTextureIndex_ = i;
					removeTexture();
				}
				ImGui::EndPopup();
			}
		}
	}

	ImGui::End();
}

void UserInterface::cloneSprite()
{
	Sprite *selectedSprite = theSpriteMgr->sprites()[selectedSpriteIndex_].get();
	theSpriteMgr->sprites().insertAt(++selectedSpriteIndex_, nctl::move(selectedSprite->clone()));
}

void UserInterface::removeSprite()
{
	Sprite *selectedSprite = theSpriteMgr->sprites()[selectedSpriteIndex_].get();
	updateSelectedAnimOnSpriteRemoval(selectedSprite);

	theAnimMgr->removeSprite(selectedSprite);
	if (selectedSpriteIndex_ >= 0 && selectedSpriteIndex_ < theSpriteMgr->sprites().size())
		theSpriteMgr->sprites().removeAt(selectedSpriteIndex_);
	if (selectedSpriteIndex_ > 0)
		selectedSpriteIndex_--;
}

void UserInterface::createSpritesWindow()
{
	ImGui::Begin(Labels::Sprites);

	if (theSpriteMgr->textures().isEmpty() == false)
	{
		if (ImGui::Button(Labels::Add))
		{
			if (selectedTextureIndex_ >= 0 && selectedTextureIndex_ < theSpriteMgr->textures().size())
			{
				if (theSpriteMgr->sprites().isEmpty())
					numFrames = 0; // force focus on the canvas

				Texture &tex = *theSpriteMgr->textures()[selectedTextureIndex_];
				theSpriteMgr->sprites().pushBack(nctl::makeUnique<Sprite>(&tex));
				selectedSpriteIndex_ = theSpriteMgr->sprites().size() - 1;
			}
		}
		if (theSpriteMgr->sprites().isEmpty() == false)
		{
			ImGui::SameLine();
			if (ImGui::Button(Labels::Remove) || (deleteKeyPressed && ImGui::IsWindowHovered()))
				removeSprite();
		}
		// Repeat the check after the remove button
		if (theSpriteMgr->sprites().isEmpty() == false)
		{
			ImGui::SameLine();
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
			ImGui::SameLine();
			if (ImGui::Button(Labels::Clone))
				cloneSprite();
		}
	}
	else
		ImGui::Text("Load at least one texture in order to add sprites");

	if (theSpriteMgr->sprites().isEmpty() == false)
	{
		const bool needsMoveUpButton = selectedSpriteIndex_ > 0;
		const bool needsMoveDownButton = selectedSpriteIndex_ < theSpriteMgr->sprites().size() - 1;

		if (needsMoveUpButton || needsMoveDownButton)
		{
			ImGui::SameLine();
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
		}

		if (needsMoveUpButton)
		{
			ImGui::SameLine();
			if (ImGui::Button(Labels::MoveUp))
			{
				nctl::swap(theSpriteMgr->sprites()[selectedSpriteIndex_], theSpriteMgr->sprites()[selectedSpriteIndex_ - 1]);
				selectedSpriteIndex_--;
			}
		}
		if (needsMoveDownButton)
		{
			ImGui::SameLine();
			if (ImGui::Button(Labels::MoveDown))
			{
				nctl::swap(theSpriteMgr->sprites()[selectedSpriteIndex_], theSpriteMgr->sprites()[selectedSpriteIndex_ + 1]);
				selectedSpriteIndex_++;
			}
		}
	}

	if (theSpriteMgr->sprites().isEmpty() == false)
	{
		ImGui::Separator();
		// Reverse the sprites order so that higher in the list means above as a rendering layer
		for (int i = theSpriteMgr->sprites().size() - 1; i >= 0; i--)
		{
			// Creating a group to use the whole entry line as a drag source or target
			ImGui::BeginGroup();
			Sprite &sprite = *theSpriteMgr->sprites()[i];
			ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
			if (i == selectedSpriteIndex_)
				nodeFlags |= ImGuiTreeNodeFlags_Selected;

			ui::auxString.format("%s###Sprite%d", sprite.visible ? Labels::VisibleIcon : Labels::InvisibleIcon, i);
			ImGui::Checkbox(ui::auxString.data(), &sprite.visible);
			ImGui::SameLine();
			ui::auxString.format("#%u: \"%s\" (%d x %d) %s", i, sprite.name.data(), sprite.width(), sprite.height(),
			                     &sprite.texture() == theSpriteMgr->textures()[selectedTextureIndex_].get() ? Labels::SelectedTextureIcon : "");
			ImGui::TreeNodeEx(static_cast<void *>(&sprite), nodeFlags, "%s", ui::auxString.data());

			if (ImGui::IsItemHovered() && ImGui::GetCurrentContext()->HoveredIdTimer > imageTooltipDelay)
			{
				if (sprite.texture().width() > 0 && sprite.texture().height() > 0 &&
				    sprite.texRect().w > 0 && sprite.texRect().h > 0)
				{
					const float invTextureWidth = 1.0f / sprite.texture().width();
					const float invTextureHeight = 1.0f / sprite.texture().height();
					const float aspectRatio = sprite.texRect().w / static_cast<float>(sprite.texRect().h);
					const nc::Recti texRect = sprite.flippingTexRect();
					const ImVec2 uv0(texRect.x * invTextureWidth, texRect.y * invTextureHeight);
					const ImVec2 uv1(uv0.x + texRect.w * invTextureWidth, uv0.y + texRect.h * invTextureHeight);

					float width = imageTooltipSize;
					float height = imageTooltipSize;
					if (aspectRatio > 1.0f)
						height = width / aspectRatio;
					else
						width *= aspectRatio;

					ImGui::BeginTooltip();
					ImGui::Image(sprite.texture().imguiTexId(), ImVec2(width, height), uv0, uv1);
					ImGui::EndTooltip();
				}
			}
			if (ImGui::IsItemClicked())
				selectedSpriteIndex_ = i;

			if (ImGui::BeginPopupContextItem())
			{
				const bool showSelectParent = sprite.parent() != nullptr;
				if (showSelectParent && ImGui::MenuItem(Labels::SelectParent))
					selectedSpriteIndex_ = theSpriteMgr->spriteIndex(sprite.parent());
				if (showSelectParent)
					ImGui::Separator();
				if (ImGui::MenuItem(Labels::Clone))
				{
					selectedSpriteIndex_ = i;
					cloneSprite();
				}
				if (ImGui::MenuItem(Labels::Remove))
				{
					selectedSpriteIndex_ = i;
					removeSprite();
				}
				ImGui::EndPopup();
			}

			ImGui::EndGroup();

			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
			{
				ImGui::SetDragDropPayload("SPRITE_TREENODE", &i, sizeof(unsigned int));
				ImGui::Text("%s", ui::auxString.data());
				ImGui::EndDragDropSource();
			}
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("SPRITE_TREENODE"))
				{
					IM_ASSERT(payload->DataSize == sizeof(unsigned int));
					const unsigned int dragIndex = *reinterpret_cast<const unsigned int *>(payload->Data);

					nctl::UniquePtr<Sprite> dragSprite(nctl::move(theSpriteMgr->sprites()[dragIndex]));
					theSpriteMgr->sprites().removeAt(dragIndex);
					theSpriteMgr->sprites().insertAt(i, nctl::move(dragSprite));
					selectedSpriteIndex_ = i;

					ImGui::EndDragDropTarget();
				}
			}
		}
	}

	ImGui::End();
}

void UserInterface::removeScript()
{
	Script *selectedScript = theScriptingMgr->scripts()[selectedScriptIndex_].get();
	theAnimMgr->removeScript(selectedScript);

	theScriptingMgr->scripts().removeAt(selectedScriptIndex_);
	if (selectedScriptIndex_ > 0)
		selectedScriptIndex_--;
}

void UserInterface::createScriptsWindow()
{
	ImGui::Begin(Labels::Scripts);

	const bool openBundledEnabled = ScriptsStrings::Count > 0;
	if (openBundledEnabled)
	{
		ui::comboString.clear();
		ui::comboString.append(Labels::BundledScripts);
		ui::comboString.setLength(ui::comboString.length() + 1);
		for (unsigned int i = 0; i < ScriptsStrings::Count; i++)
		{
			ui::comboString.formatAppend("%s", ScriptsStrings::Names[i]);
			ui::comboString.setLength(ui::comboString.length() + 1);
		}
		ui::comboString.setLength(ui::comboString.length() + 1);
		// Append a second '\0' to signal the end of the combo item list
		ui::comboString[ui::comboString.length() - 1] = '\0';

		static int currentComboScript = 0;
		if (ImGui::Combo("###BundledScripts", &currentComboScript, ui::comboString.data()) && currentComboScript > 0)
		{
			loadScript(nc::fs::joinPath(ui::scriptsDataDir, ScriptsStrings::Names[currentComboScript - 1]).data());
			currentComboScript = 0;
		}
	}

	if (ImGui::Button(Labels::Load))
	{
		FileDialog::config.directory = theCfg.scriptsPath;
		FileDialog::config.windowIcon = Labels::FileDialog_OpenIcon;
		FileDialog::config.windowTitle = "Load script file";
		FileDialog::config.okButton = Labels::Ok;
		FileDialog::config.selectionType = FileDialog::SelectionType::FILE;
		FileDialog::config.extensions = "lua\0\0";
		FileDialog::config.action = FileDialog::Action::LOAD_SCRIPT;
		FileDialog::config.windowOpen = true;
	}

	if (theScriptingMgr->scripts().isEmpty() == false)
	{
		ImGui::SameLine();
		if (ImGui::Button(Labels::Remove) || (deleteKeyPressed && ImGui::IsWindowHovered()))
			removeScript();

		ImGui::SameLine();
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
		ImGui::SameLine();
		if (ImGui::Button(Labels::Reload))
			reloadScript();
	}

	if (theScriptingMgr->scripts().isEmpty() == false)
	{
		ImGui::Separator();
		for (unsigned int i = 0; i < theScriptingMgr->scripts().size(); i++)
		{
			Script &script = *theScriptingMgr->scripts()[i];
			ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
			if (i == selectedScriptIndex_)
				nodeFlags |= ImGuiTreeNodeFlags_Selected;

			ui::auxString.format("#%u: \"%s\" %s", i, nc::fs::baseName(script.name().data()).data(), script.canRun() ? Labels::CheckIcon : Labels::TimesIcon);
			ImGui::TreeNodeEx(static_cast<void *>(&script), nodeFlags, "%s", ui::auxString.data());
			if (ImGui::IsItemClicked())
				selectedScriptIndex_ = i;

			if (ImGui::IsItemHovered() && script.canRun() == false)
			{
				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(450.0f);
				ImGui::TextColored(ImVec4(0.925f, 0.243f, 0.251f, 1.0f), "%s", script.errorMsg());
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();
			}

			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::MenuItem(Labels::Reload))
				{
					selectedScriptIndex_ = i;
					reloadScript();
				}
				if (ImGui::MenuItem(Labels::Remove))
				{
					selectedScriptIndex_ = i;
					removeScript();
				}
				ImGui::EndPopup();
			}
		}
	}

	ImGui::End();
}

void UserInterface::cloneAnimation(unsigned int &selectedIndex)
{
	selectedAnimation_->parent()->anims().insertAt(++selectedIndex, nctl::move(selectedAnimation_->clone()));
}

void UserInterface::removeAnimation()
{
	IAnimation *newSelection = nullptr;
	if (selectedAnimation_->parent())
	{
		nctl::Array<nctl::UniquePtr<IAnimation>> &anims = selectedAnimation_->parent()->anims();
		const int index = selectedAnimation_->indexInParent();
		if (index > 0)
			newSelection = anims[index - 1].get();
		else
		{
			if (anims.size() > 1)
				newSelection = anims[index + 1].get();
			else
				newSelection = selectedAnimation_->parent();
		}
	}

	theAnimMgr->removeAnimation(selectedAnimation_);
	selectedAnimation_ = newSelection;
}

struct DragAnimationPayload
{
	AnimationGroup &parent;
	unsigned int index;
};

IAnimation *removeAnimWithContextMenu = nullptr;

void UserInterface::createAnimationListEntry(IAnimation &anim, unsigned int index)
{
	ImGuiTreeNodeFlags nodeFlags = anim.isGroup() ? (ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_DefaultOpen)
	                                              : (ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
	if (&anim == selectedAnimation_)
		nodeFlags |= ImGuiTreeNodeFlags_Selected;

	AnimationGroup *animGroup = nullptr;
	if (anim.isGroup())
		animGroup = static_cast<AnimationGroup *>(&anim);

	// To preserve indentation no group is created
	ui::auxString.format("%s###Anim%lu", anim.enabled ? Labels::EnabledAnimIcon : Labels::DisabledAnimIcon, uintptr_t(&anim));
	ImGui::Checkbox(ui::auxString.data(), &anim.enabled);
	ImGui::SameLine();
	ui::auxString.format("#%u: ", index);
	if (anim.name.isEmpty() == false)
		ui::auxString.formatAppend("\"%s\" (", anim.name.data());
	if (anim.type() == IAnimation::Type::PARALLEL_GROUP)
		ui::auxString.formatAppend("Parallel Group (%u children)", animGroup->anims().size());
	else if (anim.type() == IAnimation::Type::SEQUENTIAL_GROUP)
		ui::auxString.formatAppend("Sequential Group (%u children)", animGroup->anims().size());
	if (anim.type() == IAnimation::Type::PROPERTY)
	{
		PropertyAnimation &propertyAnim = static_cast<PropertyAnimation &>(anim);
		ui::auxString.formatAppend("%s property", propertyAnim.propertyName());
		if (propertyAnim.sprite() != nullptr)
		{
			if (propertyAnim.sprite()->name.isEmpty() == false)
				ui::auxString.formatAppend(" for sprite \"%s\"", propertyAnim.sprite()->name.data());
			if (propertyAnim.sprite() == theSpriteMgr->sprites()[selectedSpriteIndex_].get())
				ui::auxString.formatAppend(" %s", Labels::SelectedSpriteIcon);
			if (propertyAnim.isLocked())
				ui::auxString.formatAppend(" %s", Labels::LockedAnimIcon);
		}
	}
	else if (anim.type() == IAnimation::Type::GRID)
	{
		GridAnimation &gridAnim = static_cast<GridAnimation &>(anim);
		ui::auxString.formatAppend("%s grid", (gridAnim.function() != nullptr) ? gridAnim.function()->name().data() : "None");
		if (gridAnim.sprite() != nullptr)
		{
			if (gridAnim.sprite()->name.isEmpty() == false)
				ui::auxString.formatAppend(" for sprite \"%s\"", gridAnim.sprite()->name.data());
			if (gridAnim.sprite() == theSpriteMgr->sprites()[selectedSpriteIndex_].get())
				ui::auxString.formatAppend(" %s", Labels::SelectedSpriteIcon);
			if (gridAnim.isLocked())
				ui::auxString.formatAppend(" %s", Labels::LockedAnimIcon);
		}
	}
	else if (anim.type() == IAnimation::Type::SCRIPT)
	{
		ScriptAnimation &scriptAnim = static_cast<ScriptAnimation &>(anim);
		ui::auxString.append("Script");
		if (scriptAnim.sprite() != nullptr)
		{
			if (scriptAnim.sprite()->name.isEmpty() == false)
				ui::auxString.formatAppend(" for sprite \"%s\"", scriptAnim.sprite()->name.data());
			if (scriptAnim.sprite() == theSpriteMgr->sprites()[selectedSpriteIndex_].get())
				ui::auxString.formatAppend(" %s", Labels::SelectedSpriteIcon);
			if (scriptAnim.isLocked())
				ui::auxString.formatAppend(" %s", Labels::LockedAnimIcon);
		}
	}
	if (anim.name.isEmpty() == false)
		ui::auxString.append(")");

	if (anim.state() == IAnimation::State::STOPPED)
		ui::auxString.formatAppend(" %s", Labels::StopIcon);
	else if (anim.state() == IAnimation::State::PAUSED)
		ui::auxString.formatAppend(" %s", Labels::PauseIcon);
	else if (anim.state() == IAnimation::State::PLAYING)
		ui::auxString.formatAppend(" %s", Labels::PlayIcon);

	// Force tree expansion to see the selected animation
	if (selectedAnimation_ && animGroup && (selectedAnimation_ != animGroup))
	{
		if (animGroup == selectedAnimation_->parent())
			ImGui::SetNextItemOpen(true, ImGuiCond_Always);
	}
	const bool treeIsOpen = ImGui::TreeNodeEx(static_cast<void *>(&anim), nodeFlags, "%s", ui::auxString.data());
	if (ImGui::IsItemClicked())
	{
		selectedAnimation_ = &anim;
		int spriteIndex = -1;
		if (anim.type() == IAnimation::Type::PROPERTY)
		{
			PropertyAnimation &propertyAnim = static_cast<PropertyAnimation &>(anim);
			spriteIndex = theSpriteMgr->spriteIndex(propertyAnim.sprite());
		}
		else if (anim.type() == IAnimation::Type::GRID)
		{
			GridAnimation &gridAnim = static_cast<GridAnimation &>(anim);
			spriteIndex = theSpriteMgr->spriteIndex(gridAnim.sprite());
		}
		else if (anim.type() == IAnimation::Type::SCRIPT)
		{
			ScriptAnimation &scriptAnim = static_cast<ScriptAnimation &>(anim);
			spriteIndex = theSpriteMgr->spriteIndex(scriptAnim.sprite());
		}
		if (spriteIndex >= 0)
			selectedSpriteIndex_ = spriteIndex;
	}

	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::MenuItem(Labels::Play))
		{
			selectedAnimation_ = &anim;
			selectedAnimation_->play();
		}
		if (ImGui::MenuItem(Labels::Pause))
		{
			selectedAnimation_ = &anim;
			selectedAnimation_->pause();
		}
		if (ImGui::MenuItem(Labels::Stop))
		{
			selectedAnimation_ = &anim;
			selectedAnimation_->stop();
		}
		const bool showLocked = anim.isGroup() == false;
		const bool showSelectParent = anim.parent() && anim.parent() != &theAnimMgr->animGroup();
		if (showLocked || showSelectParent)
			ImGui::Separator();
		if (showLocked)
		{
			CurveAnimation &curveAnim = static_cast<CurveAnimation &>(anim);
			bool locked = curveAnim.isLocked();
			ui::auxString.format("Locked %s", Labels::LockedAnimIcon);
			ImGui::Checkbox(ui::auxString.data(), &locked);
			curveAnim.setLocked(locked);
		}
		if (showSelectParent && ImGui::MenuItem(Labels::SelectParent))
			selectedAnimation_ = anim.parent();
		ImGui::Separator();
		if (ImGui::MenuItem(Labels::Clone))
		{
			selectedAnimation_ = &anim;
			cloneAnimation(index);
		}
		if (ImGui::MenuItem(Labels::Remove))
			removeAnimWithContextMenu = &anim;

		ImGui::EndPopup();
	}

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
	{
		DragAnimationPayload dragPayload = { *anim.parent(), index };
		ImGui::SetDragDropPayload("ANIMATION_TREENODE", &dragPayload, sizeof(DragAnimationPayload));
		ImGui::Text("%s", ui::auxString.data());
		ImGui::EndDragDropSource();
	}
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ANIMATION_TREENODE"))
		{
			IM_ASSERT(payload->DataSize == sizeof(DragAnimationPayload));
			const DragAnimationPayload &dragPayload = *reinterpret_cast<const DragAnimationPayload *>(payload->Data);

			bool dragIsPossible = true;
			if (dragPayload.parent.anims()[dragPayload.index]->isGroup())
			{
				AnimationGroup *dragAnimGroup = static_cast<AnimationGroup *>(dragPayload.parent.anims()[dragPayload.index].get());
				AnimationGroup *destParent = anim.parent();
				while (destParent != nullptr)
				{
					if (destParent == dragAnimGroup)
					{
						// Trying to drag a parent group inside a children
						dragIsPossible = false;
						break;
					}
					destParent = destParent->parent();
				}
			}

			if (dragIsPossible)
			{
				nctl::UniquePtr<IAnimation> dragAnimation(nctl::move(dragPayload.parent.anims()[dragPayload.index]));
				dragPayload.parent.anims().removeAt(dragPayload.index);
				if (anim.isGroup())
				{
					dragAnimation->setParent(animGroup);
					animGroup->anims().pushBack(nctl::move(dragAnimation));
				}
				else
				{
					dragAnimation->setParent(anim.parent());
					anim.parent()->anims().insertAt(index, nctl::move(dragAnimation));
				}
			}

			ImGui::EndDragDropTarget();
		}
	}

	if (anim.isGroup() && treeIsOpen)
	{
		for (unsigned int i = 0; i < animGroup->anims().size(); i++)
			createAnimationListEntry(*animGroup->anims()[i], i);

		ImGui::TreePop();
	}
}

void UserInterface::createAnimationsWindow()
{
	ImGui::Begin(Labels::Animations);

	static int currentComboAnimType = 0;
	ImGui::PushItemWidth(150.0f);
	ImGui::Combo("Type", &currentComboAnimType, animationTypes, IM_ARRAYSIZE(animationTypes));
	ImGui::PopItemWidth();
	ImGui::SameLine();
	if (ImGui::Button(Labels::Add))
	{
		nctl::Array<nctl::UniquePtr<IAnimation>> *anims = &theAnimMgr->anims();
		AnimationGroup *parent = &theAnimMgr->animGroup();
		if (selectedAnimation_)
		{
			if (selectedAnimation_->isGroup())
			{
				AnimationGroup *animGroup = static_cast<AnimationGroup *>(selectedAnimation_);
				anims = &animGroup->anims();
				parent = animGroup;
			}
			else if (selectedAnimation_->parent() != nullptr)
			{
				anims = &selectedAnimation_->parent()->anims();
				parent = selectedAnimation_->parent();
			}
		}
		Sprite *selectedSprite = nullptr;
		if (theSpriteMgr->sprites().isEmpty() == false &&
		    selectedSpriteIndex_ >= 0 && selectedSpriteIndex_ <= theSpriteMgr->sprites().size() - 1)
		{
			selectedSprite = theSpriteMgr->sprites()[selectedSpriteIndex_].get();
		}

		// Search the index of the selected animation in its parent
		unsigned int selectedIndex = anims->size() - 1;
		for (unsigned int i = 0; i < anims->size(); i++)
		{
			if ((*anims)[i].get() == selectedAnimation_)
			{
				selectedIndex = i;
				break;
			}
		}

		switch (currentComboAnimType)
		{
			case AnimationTypesEnum::PARALLEL_GROUP:
				anims->insertAt(++selectedIndex, nctl::makeUnique<ParallelAnimationGroup>());
				break;
			case AnimationTypesEnum::SEQUENTIAL_GROUP:
				anims->insertAt(++selectedIndex, nctl::makeUnique<SequentialAnimationGroup>());
				break;
			case AnimationTypesEnum::PROPERTY:
				anims->insertAt(++selectedIndex, nctl::makeUnique<PropertyAnimation>(selectedSprite));
				static_cast<PropertyAnimation &>(*(*anims)[selectedIndex]).setProperty(Properties::Types::NONE);
				break;
			case AnimationTypesEnum::GRID:
				anims->insertAt(++selectedIndex, nctl::makeUnique<GridAnimation>(selectedSprite));
				break;
			case AnimationTypesEnum::SCRIPT:
				Script *selectedScript = nullptr;
				if (theScriptingMgr->scripts().isEmpty() == false &&
				    selectedScriptIndex_ >= 0 && selectedScriptIndex_ <= theScriptingMgr->scripts().size() - 1)
				{
					selectedScript = theScriptingMgr->scripts()[selectedScriptIndex_].get();
				}
				anims->insertAt(++selectedIndex, nctl::makeUnique<ScriptAnimation>(selectedSprite, selectedScript));
				break;
		}
		(*anims)[selectedIndex]->setParent(parent);
		selectedAnimation_ = (*anims)[selectedIndex].get();
	}

	// Remove an animation as instructed by the context menu
	if (removeAnimWithContextMenu)
	{
		selectedAnimation_ = removeAnimWithContextMenu;
		removeAnimation();
		removeAnimWithContextMenu = nullptr;
	}

	if (selectedAnimation_ && selectedAnimation_ != &theAnimMgr->animGroup())
	{
		ImGui::SameLine();
		if (ImGui::Button(Labels::Remove) || (deleteKeyPressed && ImGui::IsWindowHovered()))
			removeAnimation();
	}

	// Search the index of the selected animation in its parent
	unsigned int selectedIndex = 0;
	if (selectedAnimation_ && selectedAnimation_->parent())
	{
		AnimationGroup *parent = selectedAnimation_->parent();
		selectedIndex = parent->anims().size() - 1;
		for (unsigned int i = 0; i < parent->anims().size(); i++)
		{
			if (parent->anims()[i].get() == selectedAnimation_)
			{
				selectedIndex = i;
				break;
			}
		}
	}

	// Repeat the check after the remove button
	if (selectedAnimation_ && selectedAnimation_ != &theAnimMgr->animGroup())
	{
		ImGui::SameLine();
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
		ImGui::SameLine();
		if (ImGui::Button(Labels::Clone))
			selectedAnimation_->parent()->anims().insertAt(++selectedIndex, nctl::move(selectedAnimation_->clone()));
	}

	if (ImGui::Button(Labels::Stop) && selectedAnimation_)
		selectedAnimation_->stop();
	ImGui::SameLine();
	if (ImGui::Button(Labels::Pause) && selectedAnimation_)
		selectedAnimation_->pause();
	ImGui::SameLine();
	if (ImGui::Button(Labels::Play) && selectedAnimation_)
		selectedAnimation_->play();

	if (selectedAnimation_ && selectedAnimation_ != &theAnimMgr->animGroup())
	{
		const bool needsMoveUpButton = selectedIndex > 0;
		const bool needsMoveDownButton = selectedIndex < selectedAnimation_->parent()->anims().size() - 1;

		if (needsMoveUpButton || needsMoveDownButton)
		{
			ImGui::SameLine();
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
		}

		if (needsMoveUpButton)
		{
			ImGui::SameLine();
			if (ImGui::Button(Labels::MoveUp))
				nctl::swap(selectedAnimation_->parent()->anims()[selectedIndex], selectedAnimation_->parent()->anims()[selectedIndex - 1]);
		}
		if (needsMoveDownButton)
		{
			ImGui::SameLine();
			if (ImGui::Button(Labels::MoveDown))
				nctl::swap(selectedAnimation_->parent()->anims()[selectedIndex], selectedAnimation_->parent()->anims()[selectedIndex + 1]);
		}
	}

	ImGui::Separator();

	// Special animation list entry for the root animation group in the animation manager
	ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_DefaultOpen;
	if (selectedAnimation_ == &theAnimMgr->animGroup())
		nodeFlags |= ImGuiTreeNodeFlags_Selected;

	ui::auxString.format("Root (%u children)", theAnimMgr->anims().size());
	if (theAnimMgr->animGroup().state() == IAnimation::State::STOPPED)
		ui::auxString.formatAppend(" %s", Labels::StopIcon);
	else if (theAnimMgr->animGroup().state() == IAnimation::State::PAUSED)
		ui::auxString.formatAppend(" %s", Labels::PauseIcon);
	else if (theAnimMgr->animGroup().state() == IAnimation::State::PLAYING)
		ui::auxString.formatAppend(" %s", Labels::PlayIcon);

	// Force tree expansion to see the selected animation
	if (selectedAnimation_ && selectedAnimation_ != &theAnimMgr->animGroup())
		ImGui::SetNextItemOpen(true, ImGuiCond_Always);
	const bool treeIsOpen = ImGui::TreeNodeEx(static_cast<void *>(&theAnimMgr->animGroup()), nodeFlags, "%s", ui::auxString.data());
	if (ImGui::IsItemClicked())
		selectedAnimation_ = &theAnimMgr->animGroup();

	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::MenuItem(Labels::Play))
		{
			selectedAnimation_ = &theAnimMgr->animGroup();
			theAnimMgr->animGroup().play();
		}
		if (ImGui::MenuItem(Labels::Pause))
		{
			selectedAnimation_ = &theAnimMgr->animGroup();
			theAnimMgr->animGroup().pause();
		}
		if (ImGui::MenuItem(Labels::Stop))
		{
			selectedAnimation_ = &theAnimMgr->animGroup();
			theAnimMgr->animGroup().stop();
		}
		ImGui::EndPopup();
	}

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ANIMATION_TREENODE"))
		{
			IM_ASSERT(payload->DataSize == sizeof(DragAnimationPayload));
			const DragAnimationPayload &dragPayload = *reinterpret_cast<const DragAnimationPayload *>(payload->Data);

			nctl::UniquePtr<IAnimation> dragAnimation(nctl::move(dragPayload.parent.anims()[dragPayload.index]));
			dragPayload.parent.anims().removeAt(dragPayload.index);

			dragAnimation->setParent(&theAnimMgr->animGroup());
			theAnimMgr->anims().pushBack(nctl::move(dragAnimation));

			selectedAnimation_ = dragAnimation.get();

			ImGui::EndDragDropTarget();
		}
	}

	if (treeIsOpen)
	{
		for (unsigned int i = 0; i < theAnimMgr->anims().size(); i++)
			createAnimationListEntry(*theAnimMgr->anims()[i], i);

		ImGui::TreePop();
	}

	ImGui::End();
}

void UserInterface::createSpriteWindow()
{
	ImGui::Begin(Labels::Sprite);

	if (theSpriteMgr->sprites().isEmpty() == false)
	{
		Sprite &sprite = *theSpriteMgr->sprites()[selectedSpriteIndex_];

		Texture &tex = sprite.texture();
		ImGui::Text("Texture: %s (%dx%d)", tex.name().data(), tex.width(), tex.height());

		ImGui::InputText("Name", sprite.name.data(), Sprite::MaxNameLength,
		                 ImGuiInputTextFlags_CallbackResize, ui::inputTextCallback, &sprite.name);

		// Create an array of sprites that can be a parent of the selected one
		spriteGraph_.clear();
		spriteGraph_.pushBack(SpriteStruct(-1, nullptr));
		for (unsigned int i = 0; i < theSpriteMgr->sprites().size(); i++)
			theSpriteMgr->sprites()[i]->visited = false;
		visitSprite(sprite);
		for (unsigned int i = 0; i < theSpriteMgr->sprites().size(); i++)
		{
			if (theSpriteMgr->sprites()[i]->visited == false)
				spriteGraph_.pushBack(SpriteStruct(i, theSpriteMgr->sprites()[i].get()));
		}

		int currentParentCombo = 0; // None
		ui::comboString.clear();
		for (unsigned int i = 0; i < spriteGraph_.size(); i++)
		{
			const int index = spriteGraph_[i].index;
			const Sprite &currentSprite = *spriteGraph_[i].sprite;
			if (index < 0)
				ui::comboString.append("None");
			else
				ui::comboString.formatAppend("#%u: \"%s\" (%d x %d)", index, currentSprite.name.data(), currentSprite.width(), currentSprite.height());
			ui::comboString.setLength(ui::comboString.length() + 1);

			if (sprite.parent() == &currentSprite)
				currentParentCombo = i;
		}
		ui::comboString.setLength(ui::comboString.length() + 1);
		// Append a second '\0' to signal the end of the combo item list
		ui::comboString[ui::comboString.length() - 1] = '\0';

		ImGui::Combo("Parent", &currentParentCombo, ui::comboString.data());

		const Sprite *prevParent = sprite.parent();
		const nc::Vector2f absPosition = sprite.absPosition();
		Sprite *parent = spriteGraph_[currentParentCombo].sprite;
		if (prevParent != parent)
		{
			sprite.setParent(parent);
			sprite.setAbsPosition(absPosition);
		}

		nc::Vector2f position(sprite.x, sprite.y);
		ImGui::SliderFloat2("Position", position.data(), 0.0f, static_cast<float>(theCanvas->texWidth()));
		sprite.x = roundf(position.x);
		sprite.y = roundf(position.y);
		ImGui::SliderFloat("Rotation", &sprite.rotation, 0.0f, 360.0f);
		ImGui::SliderFloat2("Scale", sprite.scaleFactor.data(), 0.0f, 8.0f);
		ImGui::SameLine();
		ui::auxString.format("%s##Scale", Labels::Reset);
		if (ImGui::Button(ui::auxString.data()))
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
		sprite.anchorPoint.x = roundf(sprite.anchorPoint.x);
		sprite.anchorPoint.y = roundf(sprite.anchorPoint.y);

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

		int minY = texRect.y;
		int maxY = minY + texRect.h;
		ImGui::DragIntRange2("Rect Y", &minY, &maxY, 1.0f, 0, tex.height());

		texRect.x = minX;
		texRect.w = maxX - minX;
		texRect.y = minY;
		texRect.h = maxY - minY;
		if (texRect.x < 0)
			texRect.x = 0;
		if (texRect.w <= 0)
			texRect.w = 1;
		if (texRect.y < 0)
			texRect.y = 0;
		if (texRect.h <= 0)
			texRect.h = 1;

		ImGui::SameLine();
		ui::auxString.format("%s##Rect", Labels::Reset);
		if (ImGui::Button(ui::auxString.data()))
			texRect = nc::Recti(0, 0, tex.width(), tex.height());

		const nc::Recti currentTexRect = sprite.texRect();
		if (texRect.x != currentTexRect.x || texRect.y != currentTexRect.y ||
		    texRect.w != currentTexRect.w || texRect.h != currentTexRect.h)
		{
			sprite.setTexRect(texRect);
		}

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
		int currentRgbBlendingPreset = static_cast<int>(sprite.rgbBlendingPreset());
		ImGui::Combo("RGB Blending", &currentRgbBlendingPreset, blendingPresets, IM_ARRAYSIZE(blendingPresets));
		sprite.setRgbBlendingPreset(static_cast<Sprite::BlendingPreset>(currentRgbBlendingPreset));

		int currentAlphaBlendingPreset = static_cast<int>(sprite.alphaBlendingPreset());
		ImGui::Combo("Alpha Blending", &currentAlphaBlendingPreset, blendingPresets, IM_ARRAYSIZE(blendingPresets));
		sprite.setAlphaBlendingPreset(static_cast<Sprite::BlendingPreset>(currentAlphaBlendingPreset));

		ImGui::ColorEdit4("Color", sprite.color.data(), ImGuiColorEditFlags_AlphaBar);
		ImGui::SameLine();
		ui::auxString.format("%s##Color", Labels::Reset);
		if (ImGui::Button(ui::auxString.data()))
			sprite.color = nc::Colorf::White;
	}

	ImGui::End();
}

void UserInterface::createDelayAnimationGui(IAnimation &anim)
{
	float delay = anim.delay();
	ImGui::SliderFloat("Delay", &delay, 0.0f, 10.0f, "%.3fs");
	if (delay < 0.0f)
		delay = 0.0f;
	anim.setDelay(delay);
	ui::auxString.format("%.3fs / %.3fs", anim.currentDelay(), anim.delay());
	if (anim.delay() > 0)
		ImGui::ProgressBar(anim.currentDelay() / anim.delay(), ImVec2(0.0f, 0.0f), ui::auxString.data());
}

void UserInterface::createLoopAnimationGui(LoopComponent &loop)
{
	int currentLoopDirection = static_cast<int>(loop.direction());
	ImGui::Combo("Direction", &currentLoopDirection, loopDirections, IM_ARRAYSIZE(loopDirections));
	loop.setDirection(static_cast<Loop::Direction>(currentLoopDirection));

	int currentLoopMode = static_cast<int>(loop.mode());
	ImGui::Combo("Loop Mode", &currentLoopMode, loopModes, IM_ARRAYSIZE(loopModes));
	loop.setMode(static_cast<Loop::Mode>(currentLoopMode));

	if (loop.mode() != Loop::Mode::DISABLED)
	{
		float loopDelay = loop.delay();
		ImGui::SliderFloat("Loop Delay", &loopDelay, 0.0f, 10.0f, "%.3fs");
		if (loopDelay < 0.0f)
			loopDelay = 0.0f;
		loop.setDelay(loopDelay);
		ui::auxString.format("%.3fs / %.3fs", loop.currentDelay(), loopDelay);
		if (loopDelay > 0.0f)
			ImGui::ProgressBar(loop.currentDelay() / loopDelay, ImVec2(0.0f, 0.0f), ui::auxString.data());
	}
}

void UserInterface::createOverrideSpriteGui(AnimationGroup &animGroup)
{
	if (animGroup.anims().isEmpty() == false)
	{
		static int currentSpriteCombo = 0;
		ui::comboString.clear();
		for (unsigned int i = 0; i < theSpriteMgr->sprites().size(); i++)
		{
			const Sprite &sprite = *theSpriteMgr->sprites()[i];
			ui::comboString.formatAppend("#%u: \"%s\" (%d x %d)", i, sprite.name.data(), sprite.width(), sprite.height());
			ui::comboString.setLength(ui::comboString.length() + 1);
		}
		ui::comboString.setLength(ui::comboString.length() + 1);
		// Append a second '\0' to signal the end of the combo item list
		ui::comboString[ui::comboString.length() - 1] = '\0';

		ImGui::Combo("Sprite##Override", &currentSpriteCombo, ui::comboString.data());
		ImGui::SameLine();
		if (ImGui::Button(Labels::Apply))
			theAnimMgr->overrideSprite(animGroup, theSpriteMgr->sprites()[currentSpriteCombo].get());
	}
}

void UserInterface::createCurveAnimationGui(CurveAnimation &anim, const CurveAnimationGuiLimits &limits)
{
	createDelayAnimationGui(anim);

	int currentComboCurveType = static_cast<int>(anim.curve().type());
	ImGui::Combo("Easing Curve", &currentComboCurveType, easingCurveTypes, IM_ARRAYSIZE(easingCurveTypes));
	anim.curve().setType(static_cast<EasingCurve::Type>(currentComboCurveType));

	createLoopAnimationGui(anim.curve().loop());

	ImGui::SliderFloat("Shift", &anim.curve().shift(), limits.minShift, limits.maxShift);
	ImGui::SameLine();
	ui::auxString.format("%s##Shift", Labels::Reset);
	if (ImGui::Button(ui::auxString.data()))
		anim.curve().shift() = 0.0f;
	ImGui::SliderFloat("Scale", &anim.curve().scale(), limits.minScale, limits.maxScale);
	ImGui::SameLine();
	ui::auxString.format("%s##Scale", Labels::Reset);
	if (ImGui::Button(ui::auxString.data()))
		anim.curve().scale() = 1.0f;

	ImGui::Separator();
	ImGui::SliderFloat("Speed", &anim.speed(), 0.0f, 5.0f);
	ImGui::SameLine();
	ui::auxString.format("%s##Speed", Labels::Reset);
	if (ImGui::Button(ui::auxString.data()))
		anim.speed() = 1.0f;

	ImGui::SliderFloat("Initial", &anim.curve().initialValue(), 0.0f, 1.0f);
	ImGui::SameLine();
	ImGui::Checkbox("##InitialEnabled", &anim.curve().hasInitialValue());

	ImGui::SliderFloat("Start", &anim.curve().start(), 0.0f, 1.0f);
	ImGui::SliderFloat("End", &anim.curve().end(), 0.0f, 1.0f);
	ImGui::SliderFloat("Time", &anim.curve().time(), anim.curve().start(), anim.curve().end());

	if (anim.curve().start() > anim.curve().end() ||
	    anim.curve().end() < anim.curve().start())
	{
		anim.curve().start() = anim.curve().end();
	}

	if (anim.curve().hasInitialValue() == false)
	{
		if (anim.curve().loop().direction() == Loop::Direction::FORWARD)
			anim.curve().setInitialValue(anim.curve().start());
		else
			anim.curve().setInitialValue(anim.curve().end());
	}
	else
	{
		if (anim.curve().initialValue() < anim.curve().start())
			anim.curve().initialValue() = anim.curve().start();
		else if (anim.curve().initialValue() > anim.curve().end())
			anim.curve().initialValue() = anim.curve().end();
	}

	if (anim.curve().time() < anim.curve().start())
		anim.curve().time() = anim.curve().start();
	else if (anim.curve().time() > anim.curve().end())
		anim.curve().time() = anim.curve().end();

	plotArray[plotValueIndex] = anim.curve().value();
	ui::auxString.format("%f", plotArray[plotValueIndex]);
	ImGui::PlotLines("Values", plotArray, PlotArraySize, 0, ui::auxString.data());
	ImGui::SameLine();
	ui::auxString.format("%s##Values", Labels::Reset);
	if (ImGui::Button(ui::auxString.data()))
	{
		plotValueIndex = 0;
		for (unsigned int i = 0; i < PlotArraySize; i++)
			plotArray[i] = 0.0f;
	}

	if (anim.state() == IAnimation::State::PLAYING)
	{
		plotValueIndex++;
		if (plotValueIndex >= PlotArraySize)
			plotValueIndex = 0;
	}
}

void UserInterface::createAnimationWindow()
{
	ImGui::Begin(Labels::Animation);

	if (theAnimMgr->anims().isEmpty() == false && selectedAnimation_ != nullptr && selectedAnimation_ != &theAnimMgr->animGroup())
	{
		IAnimation &anim = *selectedAnimation_;
		if (anim.type() == IAnimation::Type::PROPERTY)
		{
			PropertyAnimation &propertyAnim = static_cast<PropertyAnimation &>(*selectedAnimation_);
			createPropertyAnimationGui(propertyAnim);
		}
		else if (anim.type() == IAnimation::Type::GRID)
		{
			GridAnimation &gridAnim = static_cast<GridAnimation &>(*selectedAnimation_);
			createGridAnimationGui(gridAnim);
		}
		else if (anim.type() == IAnimation::Type::SCRIPT)
		{
			ScriptAnimation &scriptAnim = static_cast<ScriptAnimation &>(*selectedAnimation_);
			createScriptAnimationGui(scriptAnim);
		}
		else if (anim.type() == IAnimation::Type::SEQUENTIAL_GROUP)
		{
			SequentialAnimationGroup &sequentialAnim = static_cast<SequentialAnimationGroup &>(*selectedAnimation_);

			ImGui::InputText("Name", sequentialAnim.name.data(), IAnimation::MaxNameLength,
			                 ImGuiInputTextFlags_CallbackResize, ui::inputTextCallback, &sequentialAnim.name);

			createDelayAnimationGui(sequentialAnim);
			createLoopAnimationGui(sequentialAnim.loop());
			createOverrideSpriteGui(sequentialAnim);
		}
		else if (anim.type() == IAnimation::Type::PARALLEL_GROUP)
		{
			ParallelAnimationGroup &parallelAnim = static_cast<ParallelAnimationGroup &>(*selectedAnimation_);

			ImGui::InputText("Name", parallelAnim.name.data(), IAnimation::MaxNameLength,
			                 ImGuiInputTextFlags_CallbackResize, ui::inputTextCallback, &parallelAnim.name);

			createDelayAnimationGui(parallelAnim);
			createLoopAnimationGui(parallelAnim.loop());
			createOverrideSpriteGui(parallelAnim);
		}
	}

	ImGui::End();
}

void UserInterface::createPropertyAnimationGui(PropertyAnimation &anim)
{
	static CurveAnimationGuiLimits limits;

	ImGui::InputText("Name", anim.name.data(), IAnimation::MaxNameLength,
	                 ImGuiInputTextFlags_CallbackResize, ui::inputTextCallback, &anim.name);
	ImGui::Separator();

	if (theSpriteMgr->sprites().isEmpty() == false)
	{
		int spriteIndex = theSpriteMgr->spriteIndex(anim.sprite());
		// Assign to current sprite if none was assigned before
		if (spriteIndex < 0)
			spriteIndex = selectedSpriteIndex_;

		ui::comboString.clear();
		for (unsigned int i = 0; i < theSpriteMgr->sprites().size(); i++)
		{
			Sprite &sprite = *theSpriteMgr->sprites()[i];
			ui::comboString.formatAppend("#%u: \"%s\" (%d x %d)", i, sprite.name.data(), sprite.width(), sprite.height());
			ui::comboString.setLength(ui::comboString.length() + 1);
		}
		ui::comboString.setLength(ui::comboString.length() + 1);
		// Append a second '\0' to signal the end of the combo item list
		ui::comboString[ui::comboString.length() - 1] = '\0';

		// Also assign if no sprite was assigned before
		if (ImGui::Combo("Sprite", &spriteIndex, ui::comboString.data()) || anim.sprite() == nullptr)
		{
			anim.setSprite(theSpriteMgr->sprites()[spriteIndex].get());
			selectedSpriteIndex_ = spriteIndex;
		}

		Sprite &sprite = *theSpriteMgr->sprites()[spriteIndex];

		static int currentComboProperty = -1;
		currentComboProperty = static_cast<Properties::Types>(anim.propertyType());

		bool setCurveShift = false;
		if (ImGui::Combo("Property", &currentComboProperty, Properties::Strings, IM_ARRAYSIZE(Properties::Strings)))
			setCurveShift = true;
		anim.setProperty(static_cast<Properties::Types>(currentComboProperty));
		switch (currentComboProperty)
		{
			case Properties::Types::NONE:
				break;
			case Properties::Types::POSITION_X:
				limits.minShift = -sprite.width() * 0.5f;
				limits.maxShift = theCanvas->texWidth() + sprite.width() * 0.5f;
				limits.minScale = -theCanvas->texWidth();
				limits.maxScale = theCanvas->texWidth();
				break;
			case Properties::Types::POSITION_Y:
				limits.minShift = -sprite.height() * 0.5f;
				limits.maxShift = theCanvas->texHeight() + sprite.height() * 0.5f;
				limits.minScale = -theCanvas->texHeight();
				limits.maxScale = theCanvas->texHeight();
				break;
			case Properties::Types::ROTATION:
				limits.minShift = 0.0f;
				limits.maxShift = 360.0f;
				limits.minScale = -360.0f;
				limits.maxScale = 360.0f;
				break;
			case Properties::Types::SCALE_X:
				limits.minShift = -8.0f;
				limits.maxShift = 8.0f;
				limits.minScale = -8.0f;
				limits.maxScale = 8.0f;
				break;
			case Properties::Types::SCALE_Y:
				limits.minShift = -8.0f;
				limits.maxShift = 8.0f;
				limits.minScale = -8.0f;
				limits.maxScale = 8.0f;
				break;
			case Properties::Types::ANCHOR_X:
				limits.minShift = -sprite.width() * 0.5f;
				limits.maxShift = sprite.width() * 0.5f;
				limits.minScale = -sprite.width();
				limits.maxScale = sprite.width();
				break;
			case Properties::Types::ANCHOR_Y:
				limits.minShift = -sprite.height() * 0.5f;
				limits.maxShift = sprite.height() * 0.5f;
				limits.minScale = -sprite.height();
				limits.maxScale = sprite.height();
				break;
			case Properties::Types::OPACITY:
				limits.minShift = 0.0f;
				limits.maxShift = 1.0f;
				limits.minScale = -1.0f;
				limits.maxScale = 1.0f;
				break;
			case Properties::Types::COLOR_R:
				limits.minShift = 0.0f;
				limits.maxShift = 1.0f;
				limits.minScale = -1.0f;
				limits.maxScale = 1.0f;
				break;
			case Properties::Types::COLOR_G:
				limits.minShift = 0.0f;
				limits.maxShift = 1.0f;
				limits.minScale = -1.0f;
				limits.maxScale = 1.0f;
				break;
			case Properties::Types::COLOR_B:
				limits.minShift = 0.0f;
				limits.maxShift = 1.0f;
				limits.minScale = -1.0f;
				limits.maxScale = 1.0f;
				break;
		}
		if (setCurveShift && anim.property())
			anim.curve().setShift(*anim.property());

		ImGui::SameLine();
		bool isLocked = anim.isLocked();
		ImGui::Checkbox(Labels::Locked, &isLocked);
		anim.setLocked(isLocked);
	}
	else
		ImGui::TextDisabled("There are no sprites to animate");

	createCurveAnimationGui(anim, limits);
}

void UserInterface::createGridAnimationGui(GridAnimation &anim)
{
	CurveAnimationGuiLimits limits;
	limits.minScale = -10.0f;
	limits.maxScale = 10.0f;
	limits.minShift = -100.0f;
	limits.maxShift = 100.0f;

	ImGui::InputText("Name", anim.name.data(), IAnimation::MaxNameLength,
	                 ImGuiInputTextFlags_CallbackResize, ui::inputTextCallback, &anim.name);
	ImGui::Separator();

	if (theSpriteMgr->sprites().isEmpty() == false)
	{
		int spriteIndex = theSpriteMgr->spriteIndex(anim.sprite());
		// Assign to current sprite if none was assigned before
		if (spriteIndex < 0)
			spriteIndex = selectedSpriteIndex_;

		ui::comboString.clear();
		for (unsigned int i = 0; i < theSpriteMgr->sprites().size(); i++)
		{
			Sprite &sprite = *theSpriteMgr->sprites()[i];
			ui::comboString.formatAppend("#%u: \"%s\" (%d x %d)", i, sprite.name.data(), sprite.width(), sprite.height());
			ui::comboString.setLength(ui::comboString.length() + 1);
		}
		ui::comboString.setLength(ui::comboString.length() + 1);
		// Append a second '\0' to signal the end of the combo item list
		ui::comboString[ui::comboString.length() - 1] = '\0';

		// Also assign if no sprite was assigned before
		if (ImGui::Combo("Sprite", &spriteIndex, ui::comboString.data()) || anim.sprite() == nullptr)
		{
			anim.setSprite(theSpriteMgr->sprites()[spriteIndex].get());
			selectedSpriteIndex_ = spriteIndex;
		}

		int currentComboFunction = 0;
		ui::comboString.clear();
		ui::comboString.formatAppend("None");
		ui::comboString.setLength(ui::comboString.length() + 1);
		for (unsigned int i = 0; i < GridFunctionLibrary::gridFunctions().size(); i++)
		{
			const nctl::String &functionName = GridFunctionLibrary::gridFunctions()[i].name();
			ui::comboString.formatAppend("%s", functionName.data());
			ui::comboString.setLength(ui::comboString.length() + 1);

			if (anim.function() && functionName == anim.function()->name())
				currentComboFunction = i + 1;
		}
		ui::comboString.setLength(ui::comboString.length() + 1);
		// Append a second '\0' to signal the end of the combo item list
		ui::comboString[ui::comboString.length() - 1] = '\0';

		ImGui::Combo("Function", &currentComboFunction, ui::comboString.data());
		const GridFunction *gridFunction = (currentComboFunction > 0) ? &GridFunctionLibrary::gridFunctions()[currentComboFunction - 1] : nullptr;
		if (anim.function() != gridFunction)
			anim.setFunction(gridFunction);

		ImGui::SameLine();
		bool isLocked = anim.isLocked();
		ImGui::Checkbox(Labels::Locked, &isLocked);
		anim.setLocked(isLocked);

		if (anim.function() != nullptr)
		{
			for (unsigned int i = 0; i < anim.function()->numParameters(); i++)
			{
				const GridFunction::ParameterInfo &paramInfo = anim.function()->parameterInfo(i);
				ui::auxString.format("%s##GridFunction%u", paramInfo.name.data(), i);
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
						ImGui::SliderFloat(ui::auxString.data(), &anim.parameters()[i].value0, minValue, maxValue);
						break;
					case GridFunction::ParameterType::VECTOR2F:
						ImGui::SliderFloat2(ui::auxString.data(), &anim.parameters()[i].value0, minValue, maxValue);
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
				ui::auxString.format("%s##GridFunction%u", Labels::Reset, i);
				if (ImGui::Button(ui::auxString.data()))
				{
					anim.parameters()[i].value0 = paramInfo.initialValue.value0;
					anim.parameters()[i].value1 = paramInfo.initialValue.value1;
				}
			}
		}
	}
	else
		ImGui::TextDisabled("There are no sprites to animate");

	createCurveAnimationGui(anim, limits);
}

void UserInterface::createScriptAnimationGui(ScriptAnimation &anim)
{
	CurveAnimationGuiLimits limits;
	limits.minScale = -10.0f;
	limits.maxScale = 10.0f;
	limits.minShift = -100.0f;
	limits.maxShift = 100.0f;

	ImGui::InputText("Name", anim.name.data(), IAnimation::MaxNameLength,
	                 ImGuiInputTextFlags_CallbackResize, ui::inputTextCallback, &anim.name);
	ImGui::Separator();

	if (theSpriteMgr->sprites().isEmpty() == false)
	{
		int spriteIndex = theSpriteMgr->spriteIndex(anim.sprite());
		// Assign to current sprite if none was assigned before
		if (spriteIndex < 0)
			spriteIndex = selectedSpriteIndex_;

		ui::comboString.clear();
		for (unsigned int i = 0; i < theSpriteMgr->sprites().size(); i++)
		{
			Sprite &sprite = *theSpriteMgr->sprites()[i];
			ui::comboString.formatAppend("#%u: \"%s\" (%d x %d)", i, sprite.name.data(), sprite.width(), sprite.height());
			ui::comboString.setLength(ui::comboString.length() + 1);
		}
		ui::comboString.setLength(ui::comboString.length() + 1);
		// Append a second '\0' to signal the end of the combo item list
		ui::comboString[ui::comboString.length() - 1] = '\0';

		// Also assign if no sprite was assigned before
		if (ImGui::Combo("Sprite", &spriteIndex, ui::comboString.data()) || anim.sprite() == nullptr)
		{
			anim.setSprite(theSpriteMgr->sprites()[spriteIndex].get());
			selectedSpriteIndex_ = spriteIndex;
		}
	}
	else
		ImGui::TextDisabled("There are no sprites to animate");

	int scriptIndex = theScriptingMgr->scriptIndex(anim.script());
	if (theScriptingMgr->scripts().isEmpty() == false)
	{
		// Assign to current script if none was assigned before
		if (scriptIndex < 0)
			scriptIndex = selectedScriptIndex_;

		ui::comboString.clear();
		for (unsigned int i = 0; i < theScriptingMgr->scripts().size(); i++)
		{
			Script &script = *theScriptingMgr->scripts()[i];
			ui::comboString.formatAppend("#%u: \"%s\" %s", i, nc::fs::baseName(script.name().data()).data(),
			                             script.canRun() ? Labels::CheckIcon : Labels::TimesIcon);
			ui::comboString.setLength(ui::comboString.length() + 1);
		}
		ui::comboString.setLength(ui::comboString.length() + 1);
		// Append a second '\0' to signal the end of the combo item list
		ui::comboString[ui::comboString.length() - 1] = '\0';

		// Also assign if no script was assigned before
		if (ImGui::Combo("Script", &scriptIndex, ui::comboString.data()) || anim.script() == nullptr)
		{
			anim.setScript(theScriptingMgr->scripts()[scriptIndex].get());
			selectedScriptIndex_ = scriptIndex;
		}

		ImGui::SameLine();
		bool isLocked = anim.isLocked();
		ImGui::Checkbox(Labels::Locked, &isLocked);
		anim.setLocked(isLocked);
	}
	else
		ImGui::TextDisabled("There are no scripts to use for animation");

	createCurveAnimationGui(anim, limits);
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
					numFrames = 0; // force focus on the canvas
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
				loadTexture(selection.data());
				break;
			case FileDialog::Action::LOAD_SCRIPT:
				loadScript(selection.data());
				break;
			case FileDialog::Action::RENDER_DIR:
				renderGuiWindow_.directory = selection;
				break;
		}
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
	const float canvasZoom = canvasGuiSection_.zoomAmount();

	hoveringOnCanvasWindow = false;
	ImGui::SetNextWindowSize(ImVec2(theCanvas->texWidth() * canvasZoom, theCanvas->texHeight() * canvasZoom), ImGuiCond_Once);
	ImGui::Begin(Labels::Canvas, nullptr, ImGuiWindowFlags_HorizontalScrollbar);
	if (ImGui::IsWindowHovered())
		hoveringOnCanvasWindow = true;
	canvasGuiSection_.create(*theCanvas);

	const ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();
	ImGui::Image(theCanvas->imguiTexId(), ImVec2(theCanvas->texWidth() * canvasZoom, theCanvas->texHeight() * canvasZoom), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));

	ImDrawList *drawList = ImGui::GetWindowDrawList();
	const float lineThickness = (canvasZoom < 1.0f) ? 1.0f : canvasZoom;
	if (canvasGuiSection_.showBorders())
	{
		const ImRect borderRect(cursorScreenPos.x, cursorScreenPos.y, cursorScreenPos.x + theCanvas->texWidth() * canvasZoom, cursorScreenPos.y + theCanvas->texHeight() * canvasZoom);
		drawList->AddRect(borderRect.Min, borderRect.Max, ImColor(1.0f, 0.0f, 1.0f, 1.0f), 0.0f, ImDrawFlags_RoundCornersAll, lineThickness);
	}

	hoveringOnCanvas = false;
	if (ImGui::IsItemHovered() && theSpriteMgr->sprites().isEmpty() == false)
	{
		// Disable keyboard navigation for an easier sprite move with arrow keys
		ImGui::GetIO().ConfigFlags &= ~(ImGuiConfigFlags_NavEnableKeyboard);

		hoveringOnCanvas = true;
		const ImVec2 mousePos = ImGui::GetMousePos();
		const ImVec2 relativePos(mousePos.x - cursorScreenPos.x, mousePos.y - cursorScreenPos.y);
		Sprite &sprite = *theSpriteMgr->sprites()[selectedSpriteIndex_];
		const nc::Vector2f newSpriteAbsPos(roundf(relativePos.x / canvasZoom), roundf(relativePos.y / canvasZoom));
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

				const ImRect spriteRect(cursorScreenPos.x + (sprite.absPosition().x - sprite.absWidth() / 2) * canvasZoom,
				                        cursorScreenPos.y + (sprite.absPosition().y - sprite.absHeight() / 2) * canvasZoom,
				                        cursorScreenPos.x + (sprite.absPosition().x + sprite.absWidth() / 2) * canvasZoom,
				                        cursorScreenPos.y + (sprite.absPosition().y + sprite.absHeight() / 2) * canvasZoom);
				drawList->AddRect(spriteRect.Min, spriteRect.Max, color, 0.0f, ImDrawFlags_RoundCornersAll, lineThickness);
				if (spriteRect.Contains(mousePos))
				{
					drawList->AddLine(ImVec2(spriteRect.Min.x, mousePos.y), ImVec2(spriteRect.Max.x, mousePos.y), color, lineThickness);
					drawList->AddLine(ImVec2(mousePos.x, spriteRect.Min.y), ImVec2(mousePos.x, spriteRect.Max.y), color, lineThickness);
				}

				spriteProps_.save(sprite);

				const float rectHalfSize = 2.0f * canvasZoom;
				drawList->AddRectFilled(ImVec2(mousePos.x - rectHalfSize, mousePos.y - rectHalfSize), ImVec2(mousePos.x + rectHalfSize, mousePos.y + rectHalfSize), color);
				if (ImGui::GetIO().KeyCtrl &&
				    (spriteRelativePos.x != sprite.gridAnchorPoint.x || spriteRelativePos.y != sprite.gridAnchorPoint.y))
				{
					// Update grid anchor point while pressing Ctrl, clicking and moving the mouse
					sprite.gridAnchorPoint = spriteRelativePos;
					theAnimMgr->assignGridAnchorToParameters(&sprite);
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
			else if (ImGui::GetIO().KeyAlt && (ImGui::IsMouseDragging(ImGuiMouseButton_Left, 1.0f) || ImGui::IsMouseReleased(ImGuiMouseButton_Left)))
			{
				// Update sprite position while pressing Alt, clicking and moving the mouse (with pixel snapping)
				if (sprite.absPosition().x != newSpriteAbsPos.x || sprite.absPosition().y != newSpriteAbsPos.y)
					sprite.setAbsPosition(newSpriteAbsPos);
			}

			shiftAndClick = false;
			ctrlAndClick = false;
		}
	}
	else
	{
		// Enable keyboard navigation again
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	}

	mouseWheelCanvasZoom();

	ImGui::End();
}

void UserInterface::createTexRectWindow()
{
	const float canvasZoom = canvasGuiSection_.zoomAmount();
	static MouseStatus mouseStatus_ = MouseStatus::IDLE;
	static ImVec2 startPos(0.0f, 0.0f);
	static ImVec2 endPos(0.0f, 0.0f);

	Sprite &sprite = *theSpriteMgr->sprites()[selectedSpriteIndex_];
	nc::Recti texRect = sprite.texRect();

	ImVec2 size = ImVec2(sprite.texture().width() * canvasZoom, sprite.texture().height() * canvasZoom);
	ImGui::SetNextWindowSize(size, ImGuiCond_Once);

	ImGui::Begin(Labels::TexRect, nullptr, ImGuiWindowFlags_HorizontalScrollbar);

	ui::auxString.format("Zoom: %.2f", canvasGuiSection_.zoomAmount());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
	if (ImGui::Button(Labels::PlusIcon))
		canvasGuiSection_.increaseZoom();
	ImGui::SameLine();
	if (ImGui::Button(Labels::MinusIcon))
		canvasGuiSection_.decreaseZoom();
	ImGui::PopStyleVar();
	ImGui::SameLine();
	if (ImGui::Button(ui::auxString.data()))
		canvasGuiSection_.resetZoom();
	ImGui::Separator();

	const ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();
	ImGui::Image(sprite.imguiTexId(), size);

	mouseWheelCanvasZoom();

	if (ImGui::IsItemHovered())
	{
		const ImVec2 mousePos = ImGui::GetMousePos();
		ImVec2 relPos(mousePos.x - cursorScreenPos.x, mousePos.y - cursorScreenPos.y);

		relPos.x = roundf(relPos.x / canvasZoom);
		relPos.y = roundf(relPos.y / canvasZoom);

		// One frame lasting message
		statusMessage_.format("Coordinates: %d, %d", static_cast<int>(relPos.x), static_cast<int>(relPos.y));
	}

	if (ImGui::IsItemHovered())
	{
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			mouseStatus_ = MouseStatus::CLICKED;
			startPos = ImGui::GetMousePos();
		}
	}

	if (ImGui::IsWindowHovered() && mouseStatus_ != MouseStatus::IDLE && mouseStatus_ != MouseStatus::RELEASED)
	{
		if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
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
		if (canvasZoom > 1.0f)
		{
			startPos.x = roundf((startPos.x - cursorScreenPos.x) / canvasZoom) * canvasZoom + cursorScreenPos.x;
			startPos.y = roundf((startPos.y - cursorScreenPos.y) / canvasZoom) * canvasZoom + cursorScreenPos.y;
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
		if (canvasZoom > 1.0f)
		{
			endPos.x = roundf((endPos.x - cursorScreenPos.x) / canvasZoom) * canvasZoom + cursorScreenPos.x;
			endPos.y = roundf((endPos.y - cursorScreenPos.y) / canvasZoom) * canvasZoom + cursorScreenPos.y;
		}
	}

	ImVec2 minRect(startPos);
	ImVec2 maxRect(endPos);

	if (mouseStatus_ == MouseStatus::IDLE || mouseStatus_ == MouseStatus::CLICKED ||
	    (!(maxRect.x - minRect.x != 0.0f && maxRect.y - minRect.y != 0.0f) && mouseStatus_ != MouseStatus::DRAGGING))
	{
		// Setting the non covered rect from the sprite texrect
		minRect.x = cursorScreenPos.x + (texRect.x * canvasZoom);
		minRect.y = cursorScreenPos.y + (texRect.y * canvasZoom);
		maxRect.x = minRect.x + (texRect.w * canvasZoom);
		maxRect.y = minRect.y + (texRect.h * canvasZoom);
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
		texRect.x = (minRect.x - cursorScreenPos.x) / canvasZoom;
		texRect.y = (minRect.y - cursorScreenPos.y) / canvasZoom;
		texRect.w = (maxRect.x - minRect.x) / canvasZoom;
		texRect.h = (maxRect.y - minRect.y) / canvasZoom;
		ASSERT(texRect.x >= 0);
		ASSERT(texRect.y >= 0);

		if (texRect.w > 0 && texRect.h > 0)
			sprite.setTexRect(texRect);

		// Back to idle mouse status after assigning the new texrect
		mouseStatus_ = MouseStatus::IDLE;
	}

	ImGui::End();
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
	ImGui::TextWrapped("%s", Tips::Strings[currentTipIndex]);

	ImGui::NewLine();
	ImGui::Checkbox("Show Tips On Start", &theCfg.showTipsOnStart);
	ImGui::SameLine();
	ui::auxString.format("%s Prev", Labels::PreviousIcon);
	if (ImGui::Button(ui::auxString.data()))
	{
		if (currentTipIndex >= 1)
			currentTipIndex--;
	}
	ImGui::SameLine();
	ImGui::Text("%u / %u", currentTipIndex + 1, Tips::Count);
	ImGui::SameLine();
	ui::auxString.format("Next %s", Labels::NextIcon);
	if (ImGui::Button(ui::auxString.data()))
	{
		if (currentTipIndex <= Tips::Count - 2)
			currentTipIndex++;
	}

	ImGui::SameLine();
	if (ImGui::Button(Labels::Close))
		showTipsWindow = false;

	// Auto-save on window close
	if (showTipsWindow == false)
	{
		ui::auxString = "config.lua";
#ifdef __ANDROID__
		// On Android the configuration file is saved in the internal storage directory
		ui::auxString = nc::fs::joinPath(ui::androidCfgDir.data(), "config.lua");
#endif
		theSaver->saveCfg(ui::auxString.data(), theCfg);
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

void UserInterface::mouseWheelCanvasZoom()
{
	if (ImGui::IsWindowHovered() && ImGui::GetIO().KeyCtrl && ImGui::GetIO().MouseWheel != 0.0f)
	{
		const float wheel = ImGui::GetIO().MouseWheel;

		if (wheel > 0.0f)
			canvasGuiSection_.increaseZoom();
		else if (wheel < 0.0f)
			canvasGuiSection_.decreaseZoom();
	}
}

void UserInterface::visitSprite(Sprite &sprite)
{
	sprite.visited = true;
	for (unsigned int i = 0; i < sprite.children().size(); i++)
		visitSprite(*sprite.children()[i]);
}

void UserInterface::updateSelectedAnimOnSpriteRemoval(Sprite *sprite)
{
	if (selectedAnimation_)
	{
		if (selectedAnimation_->type() == IAnimation::Type::PROPERTY)
		{
			PropertyAnimation &propertyAnim = static_cast<PropertyAnimation &>(*selectedAnimation_);
			if (propertyAnim.sprite() == sprite)
				selectedAnimation_ = nullptr;
		}
		else if (selectedAnimation_->type() == IAnimation::Type::GRID)
		{
			GridAnimation &gridAnim = static_cast<GridAnimation &>(*selectedAnimation_);
			if (gridAnim.sprite() == sprite)
				selectedAnimation_ = nullptr;
		}
	}
}
