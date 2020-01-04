#include "Animation.h"

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

Animation::Animation()
    : Animation(EasingCurve::Type::LINEAR, EasingCurve::LoopMode::DISABLED)
{
}

Animation::Animation(EasingCurve::Type type, EasingCurve::LoopMode loopMode)
    : curve_(type, loopMode), state_(State::STOPPED), property_(nullptr), propertyName_(64)
{
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void Animation::update(float deltaTime)
{
	switch (state_)
	{
		case State::STOPPED:
		case State::PAUSED:
			if (property_)
				*property_ = curve().value();
			break;
		case State::PLAYING:
			const float value = curve_.next(deltaTime);
			if (property_)
				*property_ = value;
			break;
	}

	if (curve_.time() >= 1.0f && state_ == State::PLAYING &&
	    curve_.loopMode() == EasingCurve::LoopMode::DISABLED)
		state_ = State::STOPPED;
}

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////
