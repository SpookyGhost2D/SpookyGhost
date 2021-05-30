#include "CurveAnimation.h"
#include "AnimationGroup.h"

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
	resetDelay();
	curve_.loop().resetDelay();

	curve_.reset();
	state_ = State::STOPPED;
}

void CurveAnimation::play()
{
	if (enabled)
	{
		if (state_ == State::STOPPED)
			curve_.reset();
		state_ = State::PLAYING;
	}
}

void CurveAnimation::update(float deltaTime)
{
	switch (state_)
	{
		case State::STOPPED:
			if (curve().hasInitialValue())
				curve().setTime(curve().initialValue());

			if (parent_->state() != State::PLAYING && isLocked_)
				perform();
			break;
		case State::PAUSED:
			if (parent_->state() != State::PLAYING && isLocked_)
				perform();
			break;
		case State::PLAYING:
			if (shouldWaitDelay(deltaTime))
				return;

			if (insideSequential() && curve_.loop().mode() != Loop::Mode::DISABLED)
			{
				// Disable looping if the animation is inside a sequential group
				const Loop::Mode loopMode = curve_.loop().mode();
				curve_.loop().setMode(Loop::Mode::DISABLED);
				curve_.next(speed_ * deltaTime);
				curve_.loop().setMode(loopMode);
			}
			else
			{
				if (curve_.loop().shouldWaitDelay(deltaTime) == false)
					curve_.next(speed_ * deltaTime);
			}

			perform();

			if (curve_.loop().mode() == Loop::Mode::DISABLED)
			{
				if ((curve_.loop().direction() == Loop::Direction::FORWARD && curve().time() >= curve().end()) ||
				    (curve_.loop().direction() == Loop::Direction::BACKWARD && curve().time() <= curve().start()))
				{
					resetDelay();
					curve_.loop().resetDelay();
					state_ = State::STOPPED;
				}
			}
			break;
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
	other.curve_.loop().resetDelay();
	other.speed_ = speed_;
}
