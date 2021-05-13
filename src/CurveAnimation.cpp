#include "CurveAnimation.h"

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

CurveAnimation::CurveAnimation()
    : CurveAnimation(EasingCurve::Type::LINEAR, Loop::Mode::DISABLED)
{
}

CurveAnimation::CurveAnimation(EasingCurve::Type type, Loop::Mode loopMode)
    : isLocked_(true), curve_(type, loopMode), speed_(1.0f)
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
	if (state_ == State::PLAYING && curve_.loop().mode() == Loop::Mode::DISABLED)
	{
		if ((curve_.loop().direction() == Loop::Direction::FORWARD && curve().time() >= curve().end()) ||
		    (curve_.loop().direction() == Loop::Direction::BACKWARD && curve().time() <= curve().start()))
		{
			state_ = State::STOPPED;
		}
	}
}

///////////////////////////////////////////////////////////
// PROTECTED FUNCTIONS
///////////////////////////////////////////////////////////

void CurveAnimation::cloneTo(CurveAnimation &other) const
{
	IAnimation::cloneTo(other);

	// Always disable locking for cloned animations
	other.isLocked_ = false;
	other.curve_ = curve_;
	other.speed_ = speed_;
}
