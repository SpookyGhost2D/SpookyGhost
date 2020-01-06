#ifndef CLASS_SEQUENTIALANIMATIONGROUP
#define CLASS_SEQUENTIALANIMATIONGROUP

#include "AnimationGroup.h"

/// The sequential animation group class
class SequentialAnimationGroup : public AnimationGroup
{
  public:
	SequentialAnimationGroup() {}

	inline Type type() const override { return Type::SEQUENTIAL_GROUP; }

	void pause() override;
	void play() override;

	void update(float deltaTime) override;

  private:
	int lastPlaying_ = -1;
};

#endif
