#ifndef CLASS_PARALLELANIMATIONGROUP
#define CLASS_PARALLELANIMATIONGROUP

#include "AnimationGroup.h"

/// The parallel animation group
class ParallelAnimationGroup : public AnimationGroup
{
  public:
	nctl::UniquePtr<IAnimation> clone() const override;

	inline Type type() const override { return Type::PARALLEL_GROUP; }

	void stop() override;
	void pause() override;
	void play() override;

	void update(float deltaTime) override;
};

#endif
