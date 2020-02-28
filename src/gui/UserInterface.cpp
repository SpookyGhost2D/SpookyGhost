#include "gui/gui_common.h"
#include <ncine/imgui_internal.h>
#include <ncine/Application.h>
#include <ncine/IFile.h>

#include "singletons.h"
#include "gui/gui_labels.h"
#include "gui/UserInterface.h"
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
#include "LuaSaver.h"

#include "version.h"
#include <ncine/version.h>
#include "script_strings.h"

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
// clang-format on

const char* docsFile = "../docs/documentation.html";

static bool requestCloseModal = false;
static bool openModal = false;
static bool saveAsModal = false;
static bool allowOverwrite = false;

static bool showAboutWindow = false;
static bool showTexrectWindow = false;
static bool hoveringOnCanvas = false;

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

}

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

UserInterface::UserInterface()
    : selectedSpriteIndex_(0), spriteGraph_(4), renderGuiSection_(*this)
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

	canvasGuiSection_.setResize(theCanvas->size());

	if (theCfg.startupScriptName.isEmpty() == false)
	{
		const nctl::String startupScript = theCfg.scriptsPath + theCfg.startupScriptName;
		if (nc::IFile::access(startupScript.data(), nc::IFile::AccessMode::READABLE))
		{
			openProject(startupScript.data());
			filename_ = theCfg.startupScriptName;
		}
	}
	if (theCfg.autoPlayOnStart)
		theAnimMgr->play();
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
	return renderGuiSection_.saveAnimStatus();
}

bool UserInterface::shouldSaveFrames() const
{
	return renderGuiSection_.shouldSaveFrames();
}

bool UserInterface::shouldSaveSpritesheet() const
{
	return renderGuiSection_.shouldSaveSpritesheet();
}

void UserInterface::signalFrameSaved()
{
	renderGuiSection_.signalFrameSaved();
}

void UserInterface::cancelRender()
{
	renderGuiSection_.cancelRender();
}

void UserInterface::closeModalsAndAbout()
{
	showAboutWindow = false;
	requestCloseModal = true;
}

void UserInterface::removeSelectedSpriteWithKey()
{
	if (hoveringOnCanvas)
		removeSelectedSprite();
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
	        theSpriteMgr->sprites().isEmpty() == false ||
	        theSpriteMgr->textures().isEmpty() == false);
}

bool UserInterface::menuSaveEnabled()
{
	return (filename_.isEmpty() == false &&
	        (theSpriteMgr->textures().isEmpty() == false ||
	        theSpriteMgr->sprites().isEmpty() == false ||
	        theAnimMgr->anims().isEmpty() == false));
}

bool UserInterface::menuSaveAsEnabled()
{
	return (theSpriteMgr->textures().isEmpty() == false ||
	        theSpriteMgr->sprites().isEmpty() == false ||
	        theAnimMgr->anims().isEmpty() == false);
}

void UserInterface::menuNew()
{
	// Always clear animations before sprites
	theAnimMgr->clear();
	theSpriteMgr->sprites().clear();
	theSpriteMgr->textures().clear();
}

void UserInterface::menuOpen()
{
	openModal = true;
}

bool UserInterface::openProject(const char *filename)
{
	LuaSaver::Data data(*theCanvas, *theSpriteMgr, *theAnimMgr, renderGuiSection_.saveAnimStatus());
	if (nc::IFile::access(filename, nc::IFile::AccessMode::READABLE) &&
	    theSaver->load(filename, data))
	{
		canvasGuiSection_.setResize(theCanvas->size());
		renderGuiSection_.setResize(renderGuiSection_.saveAnimStatus().canvasResize);
		ui::auxString.format("Loaded project file \"%s\"\n",filename);
		pushStatusInfoMessage(ui::auxString.data());

		return true;
	}
	else
		return false;
}

void UserInterface::menuSave()
{
	LuaSaver::Data data(*theCanvas, *theSpriteMgr, *theAnimMgr, renderGuiSection_.saveAnimStatus());
	ui::filePath = filename_;
	if (nc::IFile::access(theCfg.scriptsPath.data(), nc::IFile::AccessMode::READABLE))
		ui::filePath = ui::joinPath(theCfg.scriptsPath, filename_);
	theSaver->save(ui::filePath.data(), data);
}

bool UserInterface::openDocumentationEnabled()
{
	nctl::String docsPath = ui::joinPath(nc::IFile::dataPath(), docsFile);
	return nc::IFile::access(docsPath.data(), nc::IFile::AccessMode::READABLE);
}

void UserInterface::openDocumentation()
{
	nctl::String docsPath = ui::joinPath(nc::IFile::dataPath(), docsFile);
	openFile(docsPath.data());
}

void UserInterface::createGui()
{
	if (lastStatus_.secondsSince() >= 2.0f)
		statusMessage_.clear();

	createDockingSpace();

	ImGui::Begin("SpookyGhost");
	canvasGuiSection_.create(*theCanvas);
	createSpritesGui();
	createAnimationsGui();
	renderGuiSection_.create();
	ImGui::End();

	createCanvasWindow();
	if (theSpriteMgr->sprites().isEmpty() == false && showTexrectWindow)
		createTexRectWindow();

	ImGui::Begin("Status");
	ImGui::Text("%s", statusMessage_.data());
	ImGui::End();

	if (showAboutWindow)
		createAboutWindow();

	createConfigWindow();
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
	createGuiPopups();

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
			LuaSaver::Data data(*theCanvas, *theSpriteMgr, *theAnimMgr, renderGuiSection_.saveAnimStatus());
			if (ImGui::MenuItem(Labels::New, "CTRL + N", false, menuNewEnabled()))
				menuNew();

			if (ImGui::MenuItem(Labels::Open, "CTRL + O"))
				openModal = true;

			const bool openBundledEnabled = ScriptStrings::Count > 0;
			if (ImGui::BeginMenu(Labels::OpenBundled, openBundledEnabled))
			{
				for (unsigned int i = 0; i < ScriptStrings::Count; i++)
				{
					if (ImGui::MenuItem(ScriptStrings::Names[i]))
					{
						nctl::String filePath = ui::joinPath(theCfg.scriptsPath, ScriptStrings::Names[i]);
						if (openProject(filePath.data()))
							filename_ = ScriptStrings::Names[i];
					}
				}
				ImGui::EndMenu();
			}

			if (ImGui::MenuItem(Labels::Save, "CTRL + S", false, menuSaveEnabled()))
				menuSave();

			if (ImGui::MenuItem(Labels::SaveAs, nullptr, false, menuSaveAsEnabled()))
				saveAsModal = true;

			if (ImGui::MenuItem(Labels::Configuration))
				showConfigWindow = true;

			ImGui::Separator();

			if (ImGui::MenuItem(Labels::Quit, "CTRL + Q"))
				nc::theApplication().quit();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help"))
		{
			if (ImGui::MenuItem(Labels::Documentation, "F1", false, openDocumentationEnabled()))
				openDocumentation();

			if (ImGui::MenuItem(Labels::About))
				showAboutWindow = true;

			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
}

void UserInterface::createGuiPopups()
{
	LuaSaver::Data data(*theCanvas, *theSpriteMgr, *theAnimMgr, renderGuiSection_.saveAnimStatus());

	if (openModal)
		ImGui::OpenPopup("Open##Modal");
	else if (saveAsModal)
		ImGui::OpenPopup("Save As##Modal");

	if (ImGui::BeginPopupModal("Open##Modal", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Enter the name of the file to load");
		ImGui::Separator();

		if (!ImGui::IsAnyItemActive())
			ImGui::SetKeyboardFocusHere();
		if (ImGui::InputText("", filename_.data(), ui::MaxStringLength,
		                     ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_AutoSelectAll,
		                     ui::inputTextCallback, &filename_) || ImGui::Button(Labels::Ok))
		{
			ui::filePath = filename_;
			if (nc::IFile::access(theCfg.scriptsPath.data(), nc::IFile::AccessMode::READABLE))
				ui::filePath = ui::joinPath(theCfg.scriptsPath, filename_);

			if (openProject(ui::filePath.data()))
				requestCloseModal = true;
			else
			{
				filename_ = Labels::LoadingError;
				ui::auxString.format("Could not load project file \"%s\"\n", ui::filePath.data());
				pushStatusErrorMessage(ui::auxString.data());
			}
		}
		ImGui::SetItemDefaultFocus();

		ImGui::SameLine();
		if (ImGui::Button(Labels::Cancel))
			requestCloseModal = true;

		if (requestCloseModal)
		{
			ImGui::CloseCurrentPopup();
			openModal = false;
			requestCloseModal = false;
		}

		ImGui::EndPopup();
	}

	if (ImGui::BeginPopupModal("Save As##Modal", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Enter the name of the file to save");
		ImGui::Separator();

		if (!ImGui::IsAnyItemActive())
			ImGui::SetKeyboardFocusHere();
		if (ImGui::InputText("", filename_.data(), ui::MaxStringLength,
		                     ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_AutoSelectAll,
		                     ui::inputTextCallback, &filename_) || ImGui::Button(Labels::Ok))
		{
			ui::filePath = filename_;
			if (nc::IFile::access(theCfg.scriptsPath.data(), nc::IFile::AccessMode::READABLE))
				ui::filePath = ui::joinPath(theCfg.scriptsPath, filename_);

			if (nc::IFile::access(ui::filePath.data(), nc::IFile::AccessMode::READABLE) && allowOverwrite == false)
			{
				filename_ = Labels::FileExists;
				ui::auxString.format("Could not overwrite existing file \"%s\"\n", ui::filePath.data());
				pushStatusErrorMessage(ui::auxString.data());
			}
			else
			{
				theSaver->save(ui::filePath.data(), data);
				requestCloseModal = true;
			}
		}
		ImGui::SetItemDefaultFocus();

		ImGui::SameLine();
		if (ImGui::Button(Labels::Cancel))
			requestCloseModal = true;
		ImGui::SameLine();
		ImGui::Checkbox("Allow Overwrite", &allowOverwrite);

		if (requestCloseModal)
		{
			ImGui::CloseCurrentPopup();
			saveAsModal = false;
			requestCloseModal = false;
		}

		ImGui::EndPopup();
	}
}

void UserInterface::createSpritesGui()
{
	static int selectedTextureIndex = 0;
	if (ImGui::CollapsingHeader(Labels::Sprites))
	{
		if (theSpriteMgr->textures().isEmpty() == false)
		{
			ui::comboString.clear();
			for (unsigned int i = 0; i < theSpriteMgr->textures().size(); i++)
			{
				Texture &texture = *theSpriteMgr->textures()[i];
				ui::comboString.formatAppend("#%u: \"%s\" (%d x %d)", i, texture.name().data(), texture.width(), texture.height());
				ui::comboString.setLength(ui::comboString.length() + 1);
			}
			ui::comboString.setLength(ui::comboString.length() + 1);
			// Append a second '\0' to signal the end of the combo item list
			ui::comboString[ui::comboString.length() - 1] = '\0';

			ImGui::Combo("Texture", &selectedTextureIndex, ui::comboString.data());

			ImGui::SameLine();
			ui::auxString.format("%s##Texture", Labels::Remove);
			if (ImGui::Button(ui::auxString.data()))
			{
				for (unsigned int i = 0; i < theSpriteMgr->sprites().size(); i++)
				{
					Sprite &sprite = *theSpriteMgr->sprites()[i];
					if (&sprite.texture() == theSpriteMgr->textures()[selectedTextureIndex].get())
					{
						theAnimMgr->removeSprite(&sprite);
						theSpriteMgr->sprites().removeAt(i);
					}
				}
				theSpriteMgr->textures().removeAt(selectedTextureIndex);
				if (selectedTextureIndex > 0)
					selectedTextureIndex--;
			}
		}
		ImGui::InputText("Filename", texFilename_.data(), ui::MaxStringLength,
		                 ImGuiInputTextFlags_CallbackResize, ui::inputTextCallback, &texFilename_);
		ImGui::SameLine();
		if (ImGui::Button(Labels::Load) && texFilename_.isEmpty() == false)
		{
			ui::filePath = texFilename_;
			if (nc::IFile::access(ui::filePath.data(), nc::IFile::AccessMode::READABLE) == false)
				ui::filePath = ui::joinPath(theCfg.texturesPath, texFilename_);
			if (nc::IFile::access(ui::filePath.data(), nc::IFile::AccessMode::READABLE))
			{
				theSpriteMgr->textures().pushBack(nctl::makeUnique<Texture>(ui::filePath.data()));
				ui::auxString.format("Loaded texture \"%s\"", ui::filePath.data());
				selectedTextureIndex = theSpriteMgr->textures().size() - 1;
			}
			else
				ui::auxString.format("Cannot load texture \"%s\"", ui::filePath.data());

			pushStatusInfoMessage(ui::auxString.data());
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		if (theSpriteMgr->textures().isEmpty() == false)
		{
			if (ImGui::Button(Labels::Add))
			{
				if (selectedTextureIndex >= 0 && selectedTextureIndex < theSpriteMgr->textures().size())
				{
					Texture &tex = *theSpriteMgr->textures()[selectedTextureIndex];
					theSpriteMgr->sprites().pushBack(nctl::makeUnique<Sprite>(&tex));
					selectedSpriteIndex_ = theSpriteMgr->sprites().size() - 1;
				}
			}
			ImGui::SameLine();
			if (ImGui::Button(Labels::Remove))
				removeSelectedSprite();
		}
		else
			ImGui::Text("Load at least one texture in order to add sprites");

		if (theSpriteMgr->sprites().isEmpty() == false)
		{
			if (selectedSpriteIndex_ > 0)
			{
				ImGui::SameLine();
				if (ImGui::Button(Labels::MoveUp))
				{
					nctl::swap(theSpriteMgr->sprites()[selectedSpriteIndex_], theSpriteMgr->sprites()[selectedSpriteIndex_ - 1]);
					selectedSpriteIndex_--;
				}
			}
			if (selectedSpriteIndex_ < theSpriteMgr->sprites().size() - 1)
			{
				ImGui::SameLine();
				if (ImGui::Button(Labels::MoveDown))
				{
					nctl::swap(theSpriteMgr->sprites()[selectedSpriteIndex_], theSpriteMgr->sprites()[selectedSpriteIndex_ + 1]);
					selectedSpriteIndex_++;
				}
			}

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

			ImGui::Combo("Sprite", &selectedSpriteIndex_, ui::comboString.data());

			Sprite &sprite = *theSpriteMgr->sprites()[selectedSpriteIndex_];

			Texture &tex = sprite.texture();
			ImGui::Text("Texture: %s (%dx%d)", tex.name().data(), tex.width(), tex.height());

			ImGui::InputText("Name", sprite.name.data(), Sprite::MaxNameLength,
			                 ImGuiInputTextFlags_CallbackResize, ui::inputTextCallback, &sprite.name);
			ImGui::SameLine();
			ImGui::Checkbox("Visible", &sprite.visible);

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
			ui::auxString.format("%s##Rect", Labels::Reset);
			if (ImGui::Button(ui::auxString.data()))
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
			ui::auxString.format("%s##Color", Labels::Reset);
			if (ImGui::Button(ui::auxString.data()))
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
		ui::auxString.format("%s##Animations", Labels::Add);
		if (ImGui::Button(ui::auxString.data()))
		{
			switch (currentComboAnimType)
			{
				case AnimationTypesEnum::PARALLEL_GROUP:
					theAnimMgr->anims().pushBack(nctl::makeUnique<ParallelAnimationGroup>());
					break;
				case AnimationTypesEnum::SEQUENTIAL_GROUP:
					theAnimMgr->anims().pushBack(nctl::makeUnique<SequentialAnimationGroup>());
					break;
				case AnimationTypesEnum::PROPERTY:
					theAnimMgr->anims().pushBack(nctl::makeUnique<PropertyAnimation>());
					break;
				case AnimationTypesEnum::GRID:
					theAnimMgr->anims().pushBack(nctl::makeUnique<GridAnimation>());
					break;
			}
			theAnimMgr->anims().back()->setParent(&theAnimMgr->animGroup());
		}

		ImGui::SameLine();
		ui::auxString.format("%s##Animations", Labels::Clear);
		if (ImGui::Button(ui::auxString.data()))
			theAnimMgr->clear();
		ImGui::Separator();

		for (unsigned int i = 0; i < theAnimMgr->anims().size(); i++)
			createRecursiveAnimationsGui(theAnimMgr->animGroup(), i);
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

	ui::auxString.format("#%u: ", index);
	if (animGroup.name.isEmpty() == false)
		ui::auxString.formatAppend("\"%s\" (", animGroup.name.data());
	if (animGroup.type() == IAnimation::Type::PARALLEL_GROUP)
		ui::auxString.append("Parallel Animation");
	else if (animGroup.type() == IAnimation::Type::SEQUENTIAL_GROUP)
		ui::auxString.append("Sequential Animation");
	if (animGroup.name.isEmpty() == false)
		ui::auxString.append(")");
	ui::auxString.append("###AnimationGroup");

	if (ImGui::TreeNodeEx(ui::auxString.data(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::InputText("Name", animGroup.name.data(), IAnimation::MaxNameLength,
		                 ImGuiInputTextFlags_CallbackResize, ui::inputTextCallback, &animGroup.name);

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
			animGroup.anims().back()->setParent(&animGroup);
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

	ui::auxString.format("#%u: ", index);
	if (anim.name.isEmpty() == false)
		ui::auxString.formatAppend("\"%s\" (", anim.name.data());
	ui::auxString.formatAppend("%s property", anim.propertyName().data());
	if (anim.sprite() != nullptr && anim.sprite()->name.isEmpty() == false)
		ui::auxString.formatAppend(" for sprite \"%s\"", anim.sprite()->name.data());
	if (anim.name.isEmpty() == false)
		ui::auxString.append(")");
	ui::auxString.append("###PropertyAnimation");

	static CurveAnimationGuiLimits limits;
	if (ImGui::TreeNodeEx(ui::auxString.data(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::InputText("Name", anim.name.data(), IAnimation::MaxNameLength,
		                 ImGuiInputTextFlags_CallbackResize, ui::inputTextCallback, &anim.name);
		ImGui::Separator();

		int spriteIndex = theSpriteMgr->spriteIndex(anim.sprite());
		if (theSpriteMgr->sprites().isEmpty() == false)
		{
			if (spriteIndex < 0)
				spriteIndex = selectedSpriteIndex_;

			ui::comboString.clear();
			for (unsigned int i = 0; i < theSpriteMgr->sprites().size(); i++)
			{
				Sprite &sprite = *theSpriteMgr->sprites()[i];
				ui::comboString.formatAppend("#%u: %s (%d x %d)", i, sprite.name.data(), sprite.width(), sprite.height());
				ui::comboString.setLength(ui::comboString.length() + 1);
			}
			ui::comboString.setLength(ui::comboString.length() + 1);
			// Append a second '\0' to signal the end of the combo item list
			ui::comboString[ui::comboString.length() - 1] = '\0';

			ImGui::Combo("Sprite", &spriteIndex, ui::comboString.data());
			anim.setSprite(theSpriteMgr->sprites()[spriteIndex].get());

			Sprite &sprite = *theSpriteMgr->sprites()[spriteIndex];

			static int currentComboProperty = -1;
			const nctl::String &propertyName = anim.propertyName();
			currentComboProperty = Properties::Types::NONE;
			if (anim.property() != nullptr)
			{
				for (unsigned int i = 0; i < IM_ARRAYSIZE(Properties::Strings); i++)
				{
					if (propertyName == Properties::Strings[i])
					{
						currentComboProperty = static_cast<Properties::Types>(i);
						break;
					}
				}
			}

			bool setCurveShift = false;
			if (ImGui::Combo("Property", &currentComboProperty, Properties::Strings, IM_ARRAYSIZE(Properties::Strings)))
				setCurveShift = true;
			anim.setPropertyName(Properties::Strings[currentComboProperty]);
			Properties::assign(anim, static_cast<Properties::Types>(currentComboProperty));
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

	ui::auxString.format("#%u: ", index);
	if (anim.name.isEmpty() == false)
		ui::auxString.formatAppend("\"%s\" (", anim.name.data());
	if (anim.function() != nullptr)
		ui::auxString.formatAppend("%s grid", anim.function()->name().data());
	if (anim.sprite() != nullptr && anim.sprite()->name.isEmpty() == false)
		ui::auxString.formatAppend(" for sprite \"%s\"", anim.sprite()->name.data());
	if (anim.name.isEmpty() == false)
		ui::auxString.append(")");
	ui::auxString.append("###GridAnimation");

	CurveAnimationGuiLimits limits;
	limits.minScale = -10.0f;
	limits.maxScale = 10.0f;
	limits.minShift = -100.0f;
	limits.maxShift = 100.0f;
	if (ImGui::TreeNodeEx(ui::auxString.data(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::InputText("Name", anim.name.data(), IAnimation::MaxNameLength,
		                 ImGuiInputTextFlags_CallbackResize, ui::inputTextCallback, &anim.name);
		ImGui::Separator();

		int spriteIndex = theSpriteMgr->spriteIndex(anim.sprite());
		if (theSpriteMgr->sprites().isEmpty() == false)
		{
			if (spriteIndex < 0)
				spriteIndex = selectedSpriteIndex_;

			ui::comboString.clear();
			for (unsigned int i = 0; i < theSpriteMgr->sprites().size(); i++)
			{
				Sprite &sprite = *theSpriteMgr->sprites()[i];
				ui::comboString.formatAppend("#%u: %s (%d x %d)", i, sprite.name.data(), sprite.width(), sprite.height());
				ui::comboString.setLength(ui::comboString.length() + 1);
			}
			ui::comboString.setLength(ui::comboString.length() + 1);
			// Append a second '\0' to signal the end of the combo item list
			ui::comboString[ui::comboString.length() - 1] = '\0';

			ImGui::Combo("Sprite", &spriteIndex, ui::comboString.data());
			Sprite *sprite = theSpriteMgr->sprites()[spriteIndex].get();
			if (anim.sprite() != sprite)
				anim.setSprite(sprite);
		}
		else
			ImGui::TextDisabled("There are no sprites to animate");

		static int currentComboFunction = -1;
		ui::comboString.clear();
		for (unsigned int i = 0; i < GridFunctionLibrary::gridFunctions().size(); i++)
		{
			const nctl::String &functionName = GridFunctionLibrary::gridFunctions()[i].name();
			ui::comboString.formatAppend("%s", functionName.data());
			ui::comboString.setLength(ui::comboString.length() + 1);

			if (anim.function() && functionName == anim.function()->name())
				currentComboFunction = i;
		}
		ui::comboString.setLength(ui::comboString.length() + 1);
		// Append a second '\0' to signal the end of the combo item list
		ui::comboString[ui::comboString.length() - 1] = '\0';
		ASSERT(currentComboFunction > -1);

		ImGui::Combo("Function", &currentComboFunction, ui::comboString.data());
		const GridFunction *gridFunction = &GridFunctionLibrary::gridFunctions()[currentComboFunction];
		if (anim.function() != gridFunction)
			anim.setFunction(gridFunction);

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
	const float canvasZoom = canvasGuiSection_.zoomAmount();

	ImGui::SetNextWindowSize(ImVec2(theCanvas->texWidth() * canvasZoom, theCanvas->texHeight() * canvasZoom), ImGuiCond_Once);
	ImGui::Begin("Canvas", nullptr, ImGuiWindowFlags_HorizontalScrollbar);
	const ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();
	ImGui::Image(theCanvas->imguiTexId(), ImVec2(theCanvas->texWidth() * canvasZoom, theCanvas->texHeight() * canvasZoom), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));

	hoveringOnCanvas = false;
	if (ImGui::IsItemHovered() && theSpriteMgr->sprites().isEmpty() == false)
	{
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

				ImDrawList *drawList = ImGui::GetWindowDrawList();

				const ImRect spriteRect(cursorScreenPos.x + (sprite.absPosition().x - sprite.absWidth() / 2) * canvasZoom,
				                        cursorScreenPos.y + (sprite.absPosition().y - sprite.absHeight() / 2) * canvasZoom,
				                        cursorScreenPos.x + (sprite.absPosition().x + sprite.absWidth() / 2) * canvasZoom,
				                        cursorScreenPos.y + (sprite.absPosition().y + sprite.absHeight() / 2) * canvasZoom);
				drawList->AddRect(spriteRect.Min, spriteRect.Max, color, 0.0f, ImDrawCornerFlags_All, canvasZoom);
				if (spriteRect.Contains(mousePos))
				{
					drawList->AddLine(ImVec2(spriteRect.Min.x, mousePos.y), ImVec2(spriteRect.Max.x, mousePos.y), color, canvasZoom);
					drawList->AddLine(ImVec2(mousePos.x, spriteRect.Min.y), ImVec2(mousePos.x, spriteRect.Max.y), color, canvasZoom);
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
	const float canvasZoom = canvasGuiSection_.zoomAmount();
	static MouseStatus mouseStatus_ = MouseStatus::IDLE;
	static ImVec2 startPos(0.0f, 0.0f);
	static ImVec2 endPos(0.0f, 0.0f);

	Sprite &sprite = *theSpriteMgr->sprites()[selectedSpriteIndex_];
	nc::Recti texRect = sprite.texRect();

	ImVec2 size = ImVec2(sprite.texture().width() * canvasZoom, sprite.texture().height() * canvasZoom);
	ImGui::SetNextWindowSize(size, ImGuiCond_Once);
	ImGui::Begin("TexRect", &showTexrectWindow, ImGuiWindowFlags_HorizontalScrollbar);
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

void UserInterface::removeSelectedSprite()
{
	if (theSpriteMgr->sprites().isEmpty() == false)
	{
		theAnimMgr->removeSprite(theSpriteMgr->sprites()[selectedSpriteIndex_].get());
		if (selectedSpriteIndex_ >= 0 && selectedSpriteIndex_ < theSpriteMgr->sprites().size())
			theSpriteMgr->sprites().removeAt(selectedSpriteIndex_);
		if (selectedSpriteIndex_ > 0)
			selectedSpriteIndex_--;
	}
}
