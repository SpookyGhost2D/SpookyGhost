#ifndef CLASS_ANIMATIONSWINDOW
#define CLASS_ANIMATIONSWINDOW

class UserInterface;
class AnimationGroup;
class IAnimation;

/// The animations window class
class AnimationsWindow
{
  public:
	explicit AnimationsWindow(UserInterface &ui);

	void create();

  private:
	struct DragAnimationPayload
	{
		AnimationGroup &parent;
		unsigned int index;
	};

	UserInterface &ui_;

	void removeAnimation();
	void createAnimationListEntry(IAnimation &anim, unsigned int index, unsigned int &animId);
};

#endif
