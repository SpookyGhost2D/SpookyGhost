#ifndef CLASS_USERINTERFACE
#define CLASS_USERINTERFACE

#include <nctl/String.h>
#include <ncine/TimeStamp.h>

#include "LuaSaver.h"
#include "gui/CanvasGuiSection.h"
#include "gui/TexturesWindow.h"
#include "gui/SpritesWindow.h"
#include "gui/ScriptsWindow.h"
#include "gui/AnimationsWindow.h"
#include "gui/SpriteWindow.h"
#include "gui/AnimationWindow.h"
#include "gui/RenderWindow.h"
#include "gui/CanvasWindows.h"

class Texture;
class SpriteEntry;
class Sprite;

namespace nc = ncine;

/// The ImGui user interface class
class UserInterface
{
  public:
	UserInterface();

	void pushStatusInfoMessage(const char *message);
	void pushStatusErrorMessage(const char *message);

	const SaveAnim &saveAnimStatus() const;
	bool shouldSaveFrames() const;
	bool shouldSaveSpritesheet() const;
	void signalFrameSaved();
	void cancelRender();
	void changeScalingFactor(float factor);
	void openVideoModePopup();

	void closeModalsAndUndockables();
	void pressDeleteKey();
	void moveSprite(int xDiff, int yDiff);
	bool menuNewEnabled();
	bool menuSaveEnabled();
	bool menuSaveAsEnabled();
	bool menuQuickOpenEnabled();
	bool menuQuickSaveEnabled();
	void menuNew();
	void menuOpen();
	void menuSave();
	void menuSaveAs();
	void menuQuickOpen();
	void menuQuickSave();
	void quit();
	bool openDocumentationEnabled();
	void openDocumentation();
	void toggleAnimation();
	void reloadScript();

	void createGui();

  private:
	static bool showConfigWindow;

	SpriteEntry *selectedSpriteEntry_;
	int selectedTextureIndex_;
	int selectedScriptIndex_;
	IAnimation *selectedAnimation_;

	bool deleteKeyPressed_ = false;
	bool enableKeyboardNav_ = true;
	int numFrames_ = 0;

#ifdef WITH_FONTAWESOME
	/// Memory buffer for the FontAwesome icons font
	nctl::UniquePtr<uint8_t[]> fontFileBuffer_;
#endif

	CanvasGuiSection canvasGuiSection_;
	TexturesWindow texturesWindow_;
	SpritesWindow spritesWindow_;
	ScriptsWindow scriptsWindow_;
	AnimationsWindow animationsWindow_;
	SpriteWindow spriteWindow_;
	AnimationWindow animationWindow_;
	RenderWindow renderWindow_;
	CanvasWindows canvasWindows_;
	LuaSaver::Data saverData_;

	nctl::String lastLoadedProject_ = nctl::String(ui::MaxStringLength);
	nctl::String lastQuickSavedProject_ = nctl::String(ui::MaxStringLength);

	nctl::String statusMessage_ = nctl::String(ui::MaxStringLength);
	nc::TimeStamp lastStatus_;

	nctl::UniquePtr<Texture> spookyLogo_;
	nctl::UniquePtr<Texture> ncineLogo_;

	bool openProject(const char *filename);

	void createDockingSpace();
	void createInitialDocking();
	void createMenuBar();
	void createToolbarWindow();

	void createFileDialog();
	void createConfigWindow();
	void createTipsWindow();
	void createAboutWindow();
	void createQuitPopup();
	void createVideoModePopup();

	void applyDarkStyle();
	void sanitizeConfigValues();
	void openFile(const char *filename);

	void updateParentOnSpriteRemoval(Sprite *sprite);
	void updateSelectedAnimOnSpriteRemoval(Sprite *sprite);

	friend class TexturesWindow;
	friend class SpritesWindow;
	friend class ScriptsWindow;
	friend class AnimationsWindow;
	friend class SpriteWindow;
	friend class AnimationWindow;
	friend class RenderWindow;
	friend class CanvasWindows;
};

#endif
