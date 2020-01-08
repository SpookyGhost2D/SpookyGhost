#include "PropertyAnimation.h"

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

PropertyAnimation::PropertyAnimation()
    : PropertyAnimation(EasingCurve::Type::LINEAR, EasingCurve::LoopMode::DISABLED)
{
}

PropertyAnimation::PropertyAnimation(EasingCurve::Type type, EasingCurve::LoopMode loopMode)
    : curve_(type, loopMode), speed_(1.0f), property_(nullptr), propertyName_(64)
{
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void PropertyAnimation::stop()
{
	curve_.reset();
	state_ = State::STOPPED;
}

void PropertyAnimation::play()
{
	if (state_ == State::STOPPED)
		curve_.reset();
	state_ = State::PLAYING;
}

void PropertyAnimation::update(float deltaTime)
{
	switch (state_)
	{
		case State::STOPPED:
		case State::PAUSED:
			if (property_)
				*property_ = curve().value();
			break;
		case State::PLAYING:
			const float value = curve_.next(speed_ * deltaTime);
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
