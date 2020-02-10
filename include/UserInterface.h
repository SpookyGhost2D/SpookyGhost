#ifndef CLASS_USERINTERFACE
#define CLASS_USERINTERFACE

#include <nctl/String.h>
#include <nctl/Array.h>
#include <ncine/Vector2.h>
#include <ncine/Colorf.h>
#include <ncine/TimeStamp.h>

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
	struct SaveAnim
	{
		inline float inverseFps() const { return 1.0f / static_cast<float>(fps); }

		nctl::String filename = nctl::String(MaxStringLength);
		unsigned int numSavedFrames = 0;
		int numFrames = 60;
		int fps = 60;
		float canvasResize = 1.0f;
		nc::Vector2i sheetDestPos;
	};

	UserInterface(Canvas &canvas, Canvas &resizedCanvas, Canvas &spritesheet, SpriteManager &spriteMgr, AnimationManager &animMgr);
	const SaveAnim &saveAnimStatus() const { return saveAnimStatus_; }
	bool shouldSaveFrames() const { return shouldSaveFrames_; }
	bool shouldSaveSpritesheet() const { return shouldSaveSpritesheet_; }
	void signalFrameSaved();

	void pushStatusInfoMessage(const char *message);
	void pushStatusErrorMessage(const char *message);

	void closeAboutWindow();
	void cancelRender();
	void removeSelectedSpriteWithKey();
	void moveSprite(int xDiff, int yDiff);
	void menuNew();
	void createGui();

  private:
	static const unsigned int MaxStringLength = 256;

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

	nctl::String comboString_ = nctl::String(1024 * 2);
	nctl::String auxString_ = nctl::String(MaxStringLength);
	Canvas &canvas_;
	Canvas &resizedCanvas_;
	Canvas &spritesheet_;
	SpriteManager &spriteMgr_;
	AnimationManager &animMgr_;
	int selectedSpriteIndex_;

	/// Used to keep track of which node can be the parent of the selected one
	nctl::Array<SpriteStruct> spriteGraph_;

	nctl::String texFilename_ = nctl::String(MaxStringLength);
	nctl::String animFilename_ = nctl::String(MaxStringLength);
	SaveAnim saveAnimStatus_;
	bool shouldSaveFrames_ = false;
	bool shouldSaveSpritesheet_ = false;

	float canvasZoom_ = 1.0f;
	nc::Vector2i customCanvasSize_;

	nctl::String statusMessage_ = nctl::String(MaxStringLength);
	nc::TimeStamp lastStatus_;

	nctl::UniquePtr<Texture> spookyLogo_;
	nctl::UniquePtr<Texture> ncineLogo_;

	void createDockingSpace();
	void createInitialDocking();
	void createMenuBar();

	void createCanvasGui();
	void createSpritesGui();
	void createAnimationsGui();
	void createRenderGui();

	void createAnimationStateGui(IAnimation &anim);
	void createCurveAnimationGui(CurveAnimation &anim, const CurveAnimationGuiLimits &limits);
	void createAnimationRemoveButton(AnimationGroup &parentGroup, unsigned int index);

	void createRecursiveAnimationsGui(AnimationGroup &parentGroup, unsigned int index);
	void createAnimationGroupGui(AnimationGroup &parentGroup, unsigned int index);
	void createPropertyAnimationGui(AnimationGroup &parentGroup, unsigned int index);
	void createGridAnimationGui(AnimationGroup &parentGroup, unsigned int index);

	void createCanvasWindow();
	void createTexRectWindow();
	void createAboutWindow();

	void mouseWheelCanvasZoom();
	void visitSprite(Sprite &sprite);
	void removeSelectedSprite();
};

#endif
