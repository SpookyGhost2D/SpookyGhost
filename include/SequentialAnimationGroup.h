#ifndef CLASS_SEQUENTIALANIMATIONGROUP
#define CLASS_SEQUENTIALANIMATIONGROUP

#include "AnimationGroup.h"
#include "LoopComponent.h"

/// The sequential animation group class
class SequentialAnimationGroup : public AnimationGroup
{
  public:
	SequentialAnimationGroup();

	nctl::UniquePtr<IAnimation> clone() const override;

	inline Type type() const override { return Type::SEQUENTIAL_GROUP; }

	inline const LoopComponent &loop() const { return loop_; }
	inline LoopComponent &loop() { return loop_; }

	void stop() override;
	void pause() override;
	void play() override;

	void update(float deltaTime) override;

  private:
	LoopComponent loop_;
};

#endif
