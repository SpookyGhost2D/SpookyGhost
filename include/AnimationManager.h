#ifndef CLASS_ANIMATIONMANAGER
#define CLASS_ANIMATIONMANAGER

#include <nctl/UniquePtr.h>
#include "AnimationGroup.h"

/// The animation manager class
class AnimationManager
{
  public:
	AnimationManager();

	inline nctl::Array<nctl::UniquePtr<IAnimation>> &anims() { return (*animGroup_).anims(); }
	inline const nctl::Array<nctl::UniquePtr<IAnimation>> &anims() const { return (*animGroup_).anims(); }

	void update(float deltaTime);
	void reset();
	void clear();

  private:
	nctl::UniquePtr<AnimationGroup> animGroup_;
};

#endif
