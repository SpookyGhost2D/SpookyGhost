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
	    : state_(State::STOPPED), parent_(nullptr) {}
	virtual ~IAnimation() {}

	virtual nctl::UniquePtr<IAnimation> clone() const = 0;

	static const unsigned int MaxNameLength = 64;
	nctl::String name;
	bool enabled = true;

	virtual Type type() const = 0;
	inline State state() const { return state_; }
	inline bool isGroup() const { return type() == Type::SEQUENTIAL_GROUP || type() == Type::PARALLEL_GROUP; }

	inline virtual void stop() { state_ = State::STOPPED; }
	inline virtual void pause() { state_ = State::PAUSED; }
	inline virtual void play() { state_ = State::PLAYING; }

	virtual void update(float deltaTime) = 0;
	virtual void reset() = 0;

	inline AnimationGroup *parent() { return parent_; }
	inline const AnimationGroup *parent() const { return parent_; }
	inline void setParent(AnimationGroup *parent) { parent_ = parent; }

  protected:
	State state_;
	AnimationGroup *parent_;
};

#endif
