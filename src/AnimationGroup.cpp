#include "AnimationGroup.h"
#include "CurveAnimation.h"

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

AnimationGroup::AnimationGroup()
    : anims_(4)
{
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void AnimationGroup::stop()
{
	resetDelay();
	loop_.resetDelay();

	if (shouldReverseAnimDirection())
	{
		for (auto &&anim : anims_)
		{
			if (anim->state() != State::STOPPED)
				reverseAnimDirection(*anim);
		}
	}

	stopAnimations();
	state_ = State::STOPPED;
}

///////////////////////////////////////////////////////////
// PROTECTED FUNCTIONS
///////////////////////////////////////////////////////////

void AnimationGroup::cloneTo(AnimationGroup &other) const
{
	IAnimation::cloneTo(other);

	other.loop_ = loop_;
	other.loop_.resetDelay();

	for (auto &&anim : anims_)
	{
		other.anims_.pushBack(nctl::move(anim->clone()));
		other.anims_.back()->setParent(&other);
	}
}

void AnimationGroup::stopAnimations()
{
	if (loop_.direction() == Loop::Direction::FORWARD)
	{
		// Reverse stop to reset animations in the correct order
		for (int i = anims_.size() - 1; i >= 0; i--)
			anims_[i]->stop();
	}
	else
	{
		for (auto &&anim : anims_)
		{
			reverseAnimDirection(*anim);
			anim->stop();
			reverseAnimDirection(*anim);
		}
	}
}

bool AnimationGroup::shouldReverseAnimDirection()
{
	return ((loop_.mode() != Loop::Mode::PING_PONG && loop_.direction() == Loop::Direction::BACKWARD) ||
	        (loop_.mode() == Loop::Mode::PING_PONG && loop_.isGoingForward() == false));
}

void AnimationGroup::reverseAnimDirection(IAnimation &anim)
{
	if (anim.isGroup())
		static_cast<AnimationGroup &>(anim).loop().reverseDirection();
	else
		static_cast<CurveAnimation &>(anim).curve().loop().reverseDirection();
}
