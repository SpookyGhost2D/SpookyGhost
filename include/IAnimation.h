#ifndef CLASS_IANIMATION
#define CLASS_IANIMATION

#include <nctl/String.h>

class AnimationGroup;

/// The animation interface class
class IAnimation
{
  public:
	enum class Type
	{
		PROPERTY,
		GRID,
		SCRIPT,
		SEQUENTIAL_GROUP,
		PARALLEL_GROUP
	};

	enum class State
	{
		STOPPED,
		PAUSED,
		PLAYING
	};

	IAnimation();
	virtual ~IAnimation() {}

	virtual nctl::UniquePtr<IAnimation> clone() const = 0;

	static const unsigned int MaxNameLength = 64;
	nctl::String name;
	bool enabled = true;

	virtual Type type() const = 0;
	inline State state() const { return state_; }
	inline bool isGroup() const { return type() == Type::SEQUENTIAL_GROUP || type() == Type::PARALLEL_GROUP; }
	inline bool isStopped() const { return state_ == State::STOPPED; }
	inline bool isPaused() const { return state_ == State::PAUSED; }
	inline bool isPlaying() const { return state_ == State::PLAYING; }

	virtual void stop();
	inline virtual void pause() { state_ = State::PAUSED; }
	inline virtual void play() { state_ = State::PLAYING; }

	virtual void update(float deltaTime) = 0;

	inline AnimationGroup *parent() { return parent_; }
	inline const AnimationGroup *parent() const { return parent_; }
	inline void setParent(AnimationGroup *parent) { parent_ = parent; }

	inline float delay() const { return delay_; }
	inline void setDelay(float delay) { delay_ = delay; }
	inline float currentDelay() const { return currentDelay_; }

	inline void resetDelay() { currentDelay_ = 0.0f; }
	bool shouldWaitDelay(float deltaTime);

	int indexInParent() const;

  protected:
	State state_;
	AnimationGroup *parent_;

	/// Delay amount in seconds before an animation begins to update
	float delay_;
	/// The amount of time in seconds waited until now for the delay
	float currentDelay_;

	void cloneTo(IAnimation &other) const;

	bool insideSequential() const;
};

#endif
