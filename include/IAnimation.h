#ifndef CLASS_IANIMATION
#define CLASS_IANIMATION

#include <nctl/String.h>

/// The animation interface class
class IAnimation
{
  public:
	enum class Type
	{
		PROPERTY,
		GRID,
		SEQUENTIAL_GROUP,
		PARALLEL_GROUP
	};

	enum class State
	{
		STOPPED,
		PAUSED,
		PLAYING
	};

	IAnimation()
	    : state_(State::PAUSED) {}
	virtual ~IAnimation() {}

	virtual Type type() const = 0;
	inline State state() const { return state_; }

	inline virtual void stop() { state_ = State::STOPPED; }
	inline virtual void pause() { state_ = State::PAUSED; }
	inline virtual void play() { state_ = State::PLAYING; }

	virtual void update(float deltaTime) = 0;
	virtual void reset() = 0;

  protected:
	State state_;
};

#endif
