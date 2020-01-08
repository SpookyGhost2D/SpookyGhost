#ifndef CLASS_USERINTERFACE
#define CLASS_USERINTERFACE

#include <nctl/String.h>
#include <ncine/Vector2.h>
#include <ncine/TimeStamp.h>

class Canvas;
class AnimationManager;
class IAnimation;
class AnimationGroup;
class PropertyAnimation;
class GridAnimation;
class Sprite;

namespace nc = ncine;

/// The ImGui user interface class
class UserInterface
{
  public:
	struct SaveAnim {
		inline float inverseFps() const { return 1.0f / fps; }

		nctl::String filename = nctl::String(MaxStringLength);
		unsigned int numSavedFrames = 0;
		int numFrames = 60;
		float fps = 60.0f;
	};

	UserInterface(Canvas &canvas, AnimationManager &animMgr, Sprite &sprite);
	const SaveAnim &saveAnimStatus() const { return saveAnimStatus_; }
	bool shouldSaveAnim() const { return shouldSaveAnim_; }
	void signalFrameSaved();

	void pushStatusInfoMessage(const char *message);
	void pushStatusErrorMessage(const char *message);

	void createGui();

  private:
	static const unsigned int MaxStringLength = 256;

	nctl::String auxString_ = nctl::String(MaxStringLength);
	Canvas &canvas_;
	AnimationManager &animMgr_;
	Sprite &sprite_;

	nctl::String texFilename_ = nctl::String(MaxStringLength);
	nctl::String animFilename_ = nctl::String(MaxStringLength);
	SaveAnim saveAnimStatus_;
	bool shouldSaveAnim_ = false;

	float canvasZoom_ = 1.0f;
	nc::Vector2i customCanvasSize_;

	nctl::String statusMessage_ = nctl::String(MaxStringLength);
	nc::TimeStamp lastStatus_;

	void createDockingSpace();
	void createInitialDocking(unsigned int dockspaceId);
	void createMenuBar();

	void createCanvasGui();
	void createSpriteGui();
	void createAnimationsGui();
	void createRenderGui();

	void createRecursiveAnimationsGui(IAnimation &anim);
	void createAnimationGroupGui(AnimationGroup &animGroup);
	void createPropertyAnimationGui(PropertyAnimation &anim);
	void createGridAnimationGui(GridAnimation &anim);
};

#endif
