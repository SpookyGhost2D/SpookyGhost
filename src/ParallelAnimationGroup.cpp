#include "ParallelAnimationGroup.h"

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void ParallelAnimationGroup::pause()
{
	if (state_ != State::PLAYING)
		return;

	for (auto &&anim : anims_)
	{
		if (anim->state() == State::PLAYING)
			anim->pause();
	}

	state_ = State::PAUSED;
}

void ParallelAnimationGroup::play()
{
	if (state_ == State::PLAYING)
		return;

	if (state_ == State::STOPPED)
	{
		for (auto &&anim : anims_)
			anim->play();
	}
	else if (state_ == State::PAUSED)
	{
		for (auto &&anim : anims_)
		{
			if (anim->state() == State::PAUSED)
				anim->play();
		}
	}

	state_ = State::PLAYING;
}
