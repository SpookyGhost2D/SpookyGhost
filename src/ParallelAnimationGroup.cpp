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

void ParallelAnimationGroup::pause()
{
	if (state_ != State::PLAYING)
		return;

	for (auto &&anim : anims_)
	{
		// The parallel group pauses all playing animations
		if (anim->state() == State::PLAYING)
			anim->pause();
	}

	state_ = State::PAUSED;
}

void ParallelAnimationGroup::play()
{
	if (enabled == false)
		return;

	switch (state_)
	{
		case State::STOPPED:
			if (anims_.isEmpty() == false)
			{
				// Stop all animations to get the initial state
				stopAnimations();

				if (loop_.direction() == Loop::Direction::FORWARD)
					loop_.goForward(true);
				else
				{
					loop_.goForward(false);
					if (shouldReverseAnimDirection())
					{
						for (auto &&anim : anims_)
							reverseAnimDirection(*anim);
					}
				}

				for (auto &&anim : anims_)
				{
					if (anim->enabled)
						anim->play();
				}
			}
			break;
		case State::PAUSED:
			for (auto &&anim : anims_)
			{
				if (anim->state() == State::PAUSED)
					anim->play();
			}
			break;
		case State::PLAYING:
			break;
	}

	state_ = State::PLAYING;
}

void ParallelAnimationGroup::update(float deltaTime)
{
	if (state_ == IAnimation::State::PLAYING)
	{
		if (shouldWaitDelay(deltaTime) ||
		    (insideSequential() == false && loop_.shouldWaitDelay(deltaTime)))
		{
			return;
		}
	}

	bool allStopped = true;
	bool allDisabled = true;
	// Update all enabled animations anyway
	for (auto &&anim : anims_)
	{
		if (anim->enabled)
		{
			anim->update(deltaTime);
			allDisabled = false;
			if (anim->state() != State::STOPPED)
				allStopped = false;
		}
	}

	if (state_ == IAnimation::State::PLAYING && allStopped)
	{
		const Loop::Mode loopMode = loop_.mode();
		// Disable looping if the animation is inside a sequential group
		if (insideSequential())
			loop_.setMode(Loop::Mode::DISABLED);

		switch (loop_.mode())
		{
			case Loop::Mode::DISABLED:
				break;
			case Loop::Mode::REWIND:
				// Stop all animations to get the initial state
				stopAnimations();
				loop_.justResetNow();
				break;
			case Loop::Mode::PING_PONG:
				loop_.justResetNow();
				loop_.toggleForward();
				break;
		}

		if (loop_.mode() != Loop::Mode::DISABLED)
		{
			for (auto &&anim : anims_)
			{
				if (loop_.mode() == Loop::Mode::PING_PONG)
					reverseAnimDirection(*anim);
				if (anim->enabled)
					anim->play();
			}
		}
		else
		{
			if (shouldReverseAnimDirection())
			{
				for (auto &&anim : anims_)
					reverseAnimDirection(*anim);
			}

			resetDelay();
			state_ = State::STOPPED;
		}

		loop_.setMode(loopMode);
	}

	// Stopping a parallel group that only contains disabled animations (like a sequential group will do)
	if (allDisabled)
	{
		resetDelay();
		state_ = State::STOPPED;
	}
}
