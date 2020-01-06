#ifndef CLASS_PARALLELANIMATIONGROUP
#define CLASS_PARALLELANIMATIONGROUP

#include "AnimationGroup.h"

/// The parallel animation group
class ParallelAnimationGroup : public AnimationGroup
{
  public:
	ParallelAnimationGroup() {}

	inline Type type() const override { return Type::PARALLEL_GROUP; }

	void pause() override;
	void play() override;
};

#endif
