#include "CurveAnimation.h"

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

CurveAnimation::CurveAnimation()
    : CurveAnimation(EasingCurve::Type::LINEAR, EasingCurve::LoopMode::DISABLED)
{
}

CurveAnimation::CurveAnimation(EasingCurve::Type type, EasingCurve::LoopMode loopMode)
    : curve_(type, loopMode), speed_(1.0f)
{
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void CurveAnimation::stop()
{
	curve_.reset();
	state_ = State::STOPPED;
}

void CurveAnimation::play()
{
	if (state_ == State::STOPPED)
		curve_.reset();
	state_ = State::PLAYING;
}

void CurveAnimation::update(float deltaTime)
{
	if (curve_.time() >= 1.0f && state_ == State::PLAYING &&
	    curve_.loopMode() == EasingCurve::LoopMode::DISABLED)
	{
		state_ = State::STOPPED;
	}
}
