#include "ParallelAnimationGroup.h"

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

nctl::UniquePtr<IAnimation> ParallelAnimationGroup::clone() const
{
	nctl::UniquePtr<ParallelAnimationGroup> animGroup = nctl::makeUnique<ParallelAnimationGroup>();
	animGroup->enabled = enabled;

	for (auto &&anim : anims_)
		animGroup->anims().pushBack(nctl::move(anim->clone()));
	animGroup->setParent(parent_);

	return nctl::move(animGroup);
}

void ParallelAnimationGroup::stop()
{
	for (auto &&anim : anims_)
		anim->stop();

	state_ = State::STOPPED;
}

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
