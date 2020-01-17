#ifndef CLASS_USERINTERFACE
#define CLASS_USERINTERFACE

#include <nctl/String.h>
#include <ncine/Vector2.h>
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
	};

	UserInterface(Canvas &canvas, SpriteManager &spriteMgr, AnimationManager &animMgr);
	const SaveAnim &saveAnimStatus() const { return saveAnimStatus_; }
	bool shouldSaveAnim() const { return shouldSaveAnim_; }
	void signalFrameSaved();

	void pushStatusInfoMessage(const char *message);
	void pushStatusErrorMessage(const char *message);

	void closeAboutWindow();
	void menuNew();
	void createGui();

  private:
	static const unsigned int MaxStringLength = 256;

	nctl::String comboString_ = nctl::String(1024 * 2);
	nctl::String auxString_ = nctl::String(MaxStringLength);
	Canvas &canvas_;
	SpriteManager &spriteMgr_;
	AnimationManager &animMgr_;
	int selectedSpriteIndex_ = 0;

	nctl::String texFilename_ = nctl::String(MaxStringLength);
	nctl::String animFilename_ = nctl::String(MaxStringLength);
	SaveAnim saveAnimStatus_;
	bool shouldSaveAnim_ = false;

	float canvasZoom_ = 1.0f;
	nc::Vector2i customCanvasSize_;

	nctl::String statusMessage_ = nctl::String(MaxStringLength);
	nc::TimeStamp lastStatus_;

	nctl::UniquePtr<Texture> spookyLogo_;
	nctl::UniquePtr<Texture> ncineLogo_;

	void createDockingSpace();
	void createInitialDocking(unsigned int dockspaceId);
	void createMenuBar();

	void createCanvasGui();
	void createSpritesGui();
	void createAnimationsGui();
	void createRenderGui();

	void createAnimationStateGui(IAnimation &anim);
	void createCurveAnimationGui(CurveAnimation &anim);

	void createRecursiveAnimationsGui(IAnimation &anim);
	void createAnimationGroupGui(AnimationGroup &animGroup);
	void createPropertyAnimationGui(PropertyAnimation &anim);
	void createGridAnimationGui(GridAnimation &anim);
};

#endif
