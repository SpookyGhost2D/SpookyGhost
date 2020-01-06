#ifndef CLASS_ANIMATIONGROUP
#define CLASS_ANIMATIONGROUP

#include <nctl/Array.h>
#include "IAnimation.h"

/// The animation group class
class AnimationGroup : public IAnimation
{
  public:
	AnimationGroup()
	    : anims_(4) {}

	inline nctl::Array<nctl::UniquePtr<IAnimation>> &anims() { return anims_; }
	inline const nctl::Array<nctl::UniquePtr<IAnimation>> &anims() const { return anims_; }

	void stop() override;

	void update(float deltaTime) override;
	void reset() override;

  protected:
	nctl::Array<nctl::UniquePtr<IAnimation>> anims_;
};

#endif
