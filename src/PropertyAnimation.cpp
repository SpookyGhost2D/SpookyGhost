#include "PropertyAnimation.h"

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

PropertyAnimation::PropertyAnimation()
    : PropertyAnimation(EasingCurve::Type::LINEAR, EasingCurve::LoopMode::DISABLED)
{
}

PropertyAnimation::PropertyAnimation(EasingCurve::Type type, EasingCurve::LoopMode loopMode)
    : CurveAnimation(type, loopMode), property_(nullptr), propertyName_(64), sprite_(nullptr)
{
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void PropertyAnimation::stop()
{
	CurveAnimation::stop();
	if (property_)
		*property_ = curve().value();
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
			curve_.next(speed_ * deltaTime);
			const float value = curve_.value();
			if (property_)
				*property_ = value;
			break;
	}

	CurveAnimation::update(deltaTime);
}
