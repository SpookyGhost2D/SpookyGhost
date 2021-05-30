#ifndef CLASS_ANIMATIONGROUP
#define CLASS_ANIMATIONGROUP

#include <nctl/Array.h>
#include "IAnimation.h"
#include "LoopComponent.h"

/// The animation group abstract class
class AnimationGroup : public IAnimation
{
  public:
	AnimationGroup();

	inline nctl::Array<nctl::UniquePtr<IAnimation>> &anims() { return anims_; }
	inline const nctl::Array<nctl::UniquePtr<IAnimation>> &anims() const { return anims_; }

	inline const LoopComponent &loop() const { return loop_; }
	inline LoopComponent &loop() { return loop_; }

	void stop() override;

  protected:
	LoopComponent loop_;
	nctl::Array<nctl::UniquePtr<IAnimation>> anims_;

	void cloneTo(AnimationGroup &other) const;

	void stopAnimations();
	bool shouldReverseAnimDirection();
	void reverseAnimDirection(IAnimation &anim);
};

#endif
