#ifndef CLASS_USERINTERFACE
#define CLASS_USERINTERFACE

#include <nctl/String.h>
#include <nctl/Array.h>
#include <ncine/Vector2.h>
#include <ncine/Colorf.h>
#include <ncine/TimeStamp.h>

#include "gui/CanvasGuiSection.h"
#include "gui/RenderGuiSection.h"

class Canvas;
class SpriteManager;
class AnimationManager;
class IAnimation;
class AnimationGroup;
class CurveAnimation;
class PropertyAnimation;
class GridAnimation;
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

	void closeModalsAndAbout();
	void removeSelectedSpriteWithKey();
	void moveSprite(int xDiff, int yDiff);
	bool menuNewEnabled();
	bool menuSaveEnabled();
	bool menuSaveAsEnabled();
	void menuNew();
	bool openProject(const char *filename);
	void menuOpen();
	void menuSave();
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

	/// Used to keep track of which node can be the parent of the selected one
	nctl::Array<SpriteStruct> spriteGraph_;

	CanvasGuiSection canvasGuiSection_;
	RenderGuiSection renderGuiSection_;

	nctl::String filename_ = nctl::String(ui::MaxStringLength);
	nctl::String texFilename_ = nctl::String(ui::MaxStringLength);

	nctl::String statusMessage_ = nctl::String(ui::MaxStringLength);
	nc::TimeStamp lastStatus_;

	nctl::UniquePtr<Texture> spookyLogo_;
	nctl::UniquePtr<Texture> ncineLogo_;

	void createDockingSpace();
	void createInitialDocking();
	void createMenuBar();
	void createGuiPopups();

	void createSpritesGui();
	void createAnimationsGui();

	void createAnimationStateGui(IAnimation &anim);
	void createCurveAnimationGui(CurveAnimation &anim, const CurveAnimationGuiLimits &limits);
	void createAnimationRemoveButton(AnimationGroup &parentGroup, unsigned int index);

	void createRecursiveAnimationsGui(AnimationGroup &parentGroup, unsigned int index);
	void createAnimationGroupGui(AnimationGroup &parentGroup, unsigned int index);
	void createPropertyAnimationGui(AnimationGroup &parentGroup, unsigned int index);
	void createGridAnimationGui(AnimationGroup &parentGroup, unsigned int index);

	void createCanvasWindow();
	void createTexRectWindow();
	void createConfigWindow();
	void createAboutWindow();

	void applyDarkStyle();
	void mouseWheelCanvasZoom();
	void visitSprite(Sprite &sprite);
	void removeSelectedSprite();
	void sanitizeConfigValues();

	friend class RenderGuiSection;
};

#endif
