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
	LoopComponent(Loop::Mode mode, Loop::Direction direction);
	LoopComponent(Loop::Mode mode);
	LoopComponent();

	inline Loop::Direction direction() const { return direction_; }
	inline void setDirection(Loop::Direction direction) { direction_ = direction; }
	void reverseDirection();

	inline Loop::Mode mode() const { return mode_; }
	inline void setMode(Loop::Mode mode) { mode_ = mode; }

	inline bool isGoingForward() const { return forward_; }
	inline void goForward(bool forward) { forward_ = forward; }

	/// Returns true if the loop is starting again this frame
	inline bool hasJustReset() const { return hasJustReset_; };

	inline float delay() const { return delay_; }
	inline void setDelay(float loopDelay) { delay_ = loopDelay; }
	inline float currentDelay() const { return currentDelay_; }

	void resetDelay();
	bool shouldWaitDelay(float deltaTime);

  private:
	Loop::Mode mode_;
	Loop::Direction direction_;
	bool forward_;
	/// A flag to indicate that a loop has just reset this frame
	bool hasJustReset_;

	/// Delay amount in seconds before an animation loops again
	float delay_;
	/// The amount of time in seconds waited until now for the loop delay
	float currentDelay_;
	/// A flag indicating if an animation should wait for the loop delay
	bool waitDelay_;

	friend class EasingCurve;
	friend class SequentialAnimationGroup;
};

#endif
