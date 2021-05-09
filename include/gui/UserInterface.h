#ifndef CLASS_USERINTERFACE
#define CLASS_USERINTERFACE

#include <nctl/String.h>
#include <nctl/Array.h>
#include <ncine/Vector2.h>
#include <ncine/Colorf.h>
#include <ncine/TimeStamp.h>

#include "LuaSaver.h"
#include "gui/CanvasGuiSection.h"
#include "gui/RenderGuiWindow.h"

#ifdef __EMSCRIPTEN__
	#include <ncine/EmscriptenLocalFile.h>
#endif

class Canvas;
class SpriteManager;
class AnimationManager;
class IAnimation;
class AnimationGroup;
class CurveAnimation;
class PropertyAnimation;
class GridAnimation;
class ScriptAnimation;
class Texture;
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

	void closeModalsAndUndockables();
	void pressDeleteKey();
	void moveSprite(int xDiff, int yDiff);
	bool menuNewEnabled();
	bool menuSaveEnabled();
	bool menuSaveAsEnabled();
	void menuNew();
	void menuOpen();
	void menuSave();
	void menuSaveAs();
	bool openDocumentationEnabled();
	void openDocumentation();
	void toggleAnimation();
	void reloadScript();

	void createGui();

  private:
	static bool showConfigWindow;

	enum class MouseStatus
	{
		IDLE,
		CLICKED,
		RELEASED,
		DRAGGING
	};

	/// Used to keep track of which node can be parent of another node
	struct SpriteStruct
	{
		SpriteStruct()
		    : index(-1), sprite(nullptr) {}
		SpriteStruct(int i, Sprite *sp)
		    : index(i), sprite(sp) {}

		int index;
		Sprite *sprite;
	};

	struct CurveAnimationGuiLimits
	{
		float minScale = -1.0;
		float maxScale = 1.0f;
		float minShift = -1.0f;
		float maxShift = 1.0f;
	};

	class SpriteProperties
	{
	  public:
		void save(Sprite &sprite);
		void restore(Sprite &sprite);

	  private:
		bool saved_ = false;
		Sprite *parent_ = nullptr;
		nc::Vector2f position_ = nc::Vector2f(0.0f, 0.0f);
		float rotation_ = 0.0f;
		nc::Vector2f scaleFactor_ = nc::Vector2f(1.0f, 1.0f);
		nc::Vector2f anchorPoint_ = nc::Vector2f(1.0f, 1.0f);
		nc::Colorf color_ = nc::Colorf::White;
	};
	SpriteProperties spriteProps_;
	int selectedSpriteIndex_;
	int selectedTextureIndex_;
	int selectedScriptIndex_;
	IAnimation *selectedAnimation_;

	/// Used to keep track of which node can be the parent of the selected one
	nctl::Array<SpriteStruct> spriteGraph_;

	CanvasGuiSection canvasGuiSection_;
	RenderGuiWindow renderGuiWindow_;
	LuaSaver::Data saverData_;

#ifdef __EMSCRIPTEN__
	nc::EmscriptenLocalFile textureLocalFile_;
#endif

	nctl::String lastLoadedProject_ = nctl::String(ui::MaxStringLength);

	nctl::String statusMessage_ = nctl::String(ui::MaxStringLength);
	nc::TimeStamp lastStatus_;

	nctl::UniquePtr<Texture> spookyLogo_;
	nctl::UniquePtr<Texture> ncineLogo_;

	bool openProject(const char *filename);
	bool loadTexture(const char *filename);
	bool loadScript(const char *filename);

#ifdef __EMSCRIPTEN__
	bool loadTexture(const char *bufferName, const char *bufferPtr, unsigned long int bufferSize);
#endif

	bool loadTextureImpl(nctl::UniquePtr<Texture> texture, const char *name);

	void createDockingSpace();
	void createInitialDocking();
	void createMenuBar();

	void createToolbarWindow();
	void removeTexture();
	void createTexturesWindow();
	void cloneSprite();
	void removeSprite();
	void createSpritesWindow();
	void removeScript();
	void createScriptsWindow();
	void cloneAnimation(unsigned int &selectedIndex);
	void removeAnimation();
	void createAnimationListEntry(IAnimation &anim, unsigned int index);
	void createAnimationsWindow();

	void createSpriteWindow();
	void createAnimationWindow();

	void createCurveAnimationGui(CurveAnimation &anim, const CurveAnimationGuiLimits &limits);
	void createPropertyAnimationGui(PropertyAnimation &anim);
	void createGridAnimationGui(GridAnimation &anim);
	void createScriptAnimationGui(ScriptAnimation &anim);

	void createFileDialog();
	void createCanvasWindow();
	void createTexRectWindow();
	void createConfigWindow();
	void createAboutWindow();
	void createTipsWindow();

	void applyDarkStyle();
	void mouseWheelCanvasZoom();
	void visitSprite(Sprite &sprite);
	void sanitizeConfigValues();
	void openFile(const char *filename);

	void updateSelectedAnimOnSpriteRemoval(Sprite *sprite);

	friend class RenderGuiWindow;
};

#endif
