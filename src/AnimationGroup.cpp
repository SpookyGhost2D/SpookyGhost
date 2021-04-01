#include "AnimationGroup.h"

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void AnimationGroup::update(float deltaTime)
{
	bool allStopped = true;
	for (auto &&anim : anims_)
	{
		anim->update(deltaTime);
		if (anim->state() != State::STOPPED)
			allStopped = false;
	}
	if (allStopped)
		state_ = State::STOPPED;
}

void AnimationGroup::reset()
{
	for (auto &&anim : anims_)
		anim->reset();
}
