#include "ParallelAnimationGroup.h"

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

nctl::UniquePtr<IAnimation> ParallelAnimationGroup::clone() const
{
	nctl::UniquePtr<ParallelAnimationGroup> animGroup = nctl::makeUnique<ParallelAnimationGroup>();
	AnimationGroup::cloneTo(*animGroup);

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

void ParallelAnimationGroup::update(float deltaTime)
{
	if (state() == IAnimation::State::PLAYING)
	{
		if (shouldWaitDelay(deltaTime))
			return;
	}

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
	{
		resetDelay();
		state_ = State::STOPPED;
	}
}
