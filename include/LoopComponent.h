#ifndef CLASS_LOOPCOMPONENT
#define CLASS_LOOPCOMPONENT

namespace Loop {

enum class Direction
{
	FORWARD,
	BACKWARD
};

enum class Mode
{
	DISABLED,
	REWIND,
	PING_PONG
};

}

/// The composition class to add looping functionality
class LoopComponent
{
  public:
	LoopComponent(Loop::Mode mode, Loop::Direction direction)
	    : mode_(mode), direction_(direction), forward_(true) {}

	LoopComponent(Loop::Mode mode)
	    : LoopComponent(mode, Loop::Direction::FORWARD) {}

	LoopComponent()
	    : LoopComponent(Loop::Mode::DISABLED, Loop::Direction::FORWARD) {}

	inline Loop::Direction direction() const { return direction_; }
	inline void setDirection(Loop::Direction direction) { direction_ = direction; }

	inline Loop::Mode mode() const { return mode_; }
	inline void setMode(Loop::Mode mode) { mode_ = mode; }

	inline bool isGoingForward() const { return forward_; }
	inline void goForward(bool forward) { forward_ = forward; }

  private:
	Loop::Mode mode_;
	Loop::Direction direction_;
	bool forward_;

	friend class EasingCurve;
	friend class SequentialAnimationGroup;
};

#endif
