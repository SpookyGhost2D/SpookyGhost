#ifndef CLASS_ANIMATIONWINDOW
#define CLASS_ANIMATIONWINDOW

class UserInterface;
class IAnimation;
class AnimationGroup;
class LoopComponent;
class CurveAnimation;
class PropertyAnimation;
class GridAnimation;
class ScriptAnimation;

/// The animation window class
class AnimationWindow
{
  public:
	struct CurveAnimationGuiLimits
	{
		float minScale = -1.0;
		float maxScale = 1.0f;
		float minShift = -1.0f;
		float maxShift = 1.0f;
	};

	explicit AnimationWindow(UserInterface &ui);

	void create();

  private:
	UserInterface &ui_;

	void createDelayAnimationGui(IAnimation &anim);
	void createLoopAnimationGui(LoopComponent &loop);
	void createOverrideSpriteGui(AnimationGroup &animGroup);
	void createCurveAnimationGui(CurveAnimation &anim, const CurveAnimationGuiLimits &limits);
	void createPropertyAnimationGui(PropertyAnimation &anim);
	void createGridAnimationGui(GridAnimation &anim);
	void createScriptAnimationGui(ScriptAnimation &anim);
};

#endif
