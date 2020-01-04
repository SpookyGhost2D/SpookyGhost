#ifndef CLASS_ANIMATION
#define CLASS_ANIMATION

#include <nctl/String.h>
#include "EasingCurve.h"

/// The animation class
class Animation
{
  public:

	enum class LoopMode
	{
		DISABLED,
		REWIND,
		PING_PONG
	};

	enum class State
	{
		STOPPED,
		PAUSED,
		PLAYING
	};

	Animation();
	Animation(EasingCurve::Type curveType, EasingCurve::LoopMode loopMode);

	inline EasingCurve &curve() { return curve_; }

	inline void setProperty(float *property) { property_ = property; }
	inline const nctl::String &propertyName() const { return propertyName_; }
	inline void setPropertyName(const nctl::String &name) { propertyName_ = name; }
	//setProperty(void *fun); //TODO: with setter function

	inline State state() const { return state_; }
	inline void stop() { state_ = State::STOPPED; curve_.reset(); }
	inline void pause() { state_ = State::PAUSED; }
	inline void play() { state_ = State::PLAYING; }

	void update(float deltaTime);
	inline void reset() { curve_.reset(); }

  private:
	EasingCurve curve_;
	State state_;

	float *property_;
	nctl::String propertyName_;
};

#endif
