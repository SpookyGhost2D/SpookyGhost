#include "AnimationGroup.h"

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

void AnimationGroup::update(float deltaTime)
{
	bool allStopped = true;
	for (auto &&anim : anims_)
	{
		if (anim->enabled)
		{
			anim->update(deltaTime);
			if (anim->state() != State::STOPPED)
				allStopped = false;
		}
	}
	if (allStopped)
		state_ = State::STOPPED;
}

void AnimationGroup::reset()
{
	for (auto &&anim : anims_)
		anim->reset();
}

///////////////////////////////////////////////////////////
// PROTECTED FUNCTIONS
///////////////////////////////////////////////////////////

void AnimationGroup::cloneTo(AnimationGroup &other) const
{
	IAnimation::cloneTo(other);

	for (auto &&anim : anims_)
		other.anims_.pushBack(nctl::move(anim->clone()));
}
