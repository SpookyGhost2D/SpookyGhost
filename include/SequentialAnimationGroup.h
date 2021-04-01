#ifndef CLASS_SEQUENTIALANIMATIONGROUP
#define CLASS_SEQUENTIALANIMATIONGROUP

#include "AnimationGroup.h"

/// The sequential animation group class
class SequentialAnimationGroup : public AnimationGroup
{
  public:
	enum class Direction
	{
		FORWARD,
		BACKWARD
	};

	enum class LoopMode
	{
		DISABLED,
		REWIND,
		PING_PONG
	};

	SequentialAnimationGroup();

	inline Type type() const override { return Type::SEQUENTIAL_GROUP; }

	inline Direction direction() const { return direction_; }
	inline void setDirection(Direction direction) { direction_ = direction; }

	inline LoopMode loopMode() const { return loopMode_; }
	inline void setLoopMode(LoopMode loopMode) { loopMode_ = loopMode; }

	void stop() override;
	void pause() override;
	void play() override;

	void update(float deltaTime) override;

  private:
	Direction direction_;
	LoopMode loopMode_;
	bool forward_;
};

#endif
