#ifndef CLASS_ANIMATIONGROUP
#define CLASS_ANIMATIONGROUP

#include <nctl/Array.h>
#include "IAnimation.h"

/// The animation group abstract class
class AnimationGroup : public IAnimation
{
  public:
	AnimationGroup();

	inline nctl::Array<nctl::UniquePtr<IAnimation>> &anims() { return anims_; }
	inline const nctl::Array<nctl::UniquePtr<IAnimation>> &anims() const { return anims_; }

	void reset() override;

  protected:
	nctl::Array<nctl::UniquePtr<IAnimation>> anims_;

	void cloneTo(AnimationGroup &other) const;
};

#endif
