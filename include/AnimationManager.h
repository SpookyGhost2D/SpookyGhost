#ifndef CLASS_ANIMATIONMANAGER
#define CLASS_ANIMATIONMANAGER

#include <nctl/Array.h>
#include "Animation.h"

/// The animation manager class
class AnimationManager
{
  public:
	AnimationManager();

	inline nctl::Array<Animation> &anims() { return anims_; }
	inline const nctl::Array<Animation> &anims() const { return anims_; }

	void update(float deltaTime);
	void reset();

  private:
	nctl::Array<Animation> anims_;
};

#endif
