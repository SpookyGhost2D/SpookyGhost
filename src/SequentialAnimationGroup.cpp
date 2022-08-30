#include "SequentialAnimationGroup.h"

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

nctl::UniquePtr<IAnimation> SequentialAnimationGroup::clone() const
{
	nctl::UniquePtr<SequentialAnimationGroup> animGroup = nctl::makeUnique<SequentialAnimationGroup>();
	AnimationGroup::cloneTo(*animGroup);

	return nctl::move(animGroup);
}

void SequentialAnimationGroup::pause()
{
	if (state_ != State::PLAYING)
		return;

	for (auto &&anim : anims_)
	{
		// The sequential group pauses the first and only playing animation
		if (anim->state() == State::PLAYING)
		{
			anim->pause();
			break;
		}
	}

	state_ = State::PAUSED;
}

void SequentialAnimationGroup::play()
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
				{
					loop_.goForward(true);

					// Find first enabled animation
					unsigned int firstEnabled = 0;
					while (firstEnabled < anims_.size() && anims_[firstEnabled]->enabled == false)
						firstEnabled++;

					if (firstEnabled < anims_.size())
						anims_[firstEnabled]->play();
				}
				else
				{
					loop_.goForward(false);
					if (shouldReverseAnimDirection())
						reverseAnimDirection(*anims_.back());

					// Find last enabled animation
					int lastEnabled = anims_.size() - 1;
					while (lastEnabled >= 0 && anims_[lastEnabled]->enabled == false)
						lastEnabled--;

					if (lastEnabled >= 0)
						anims_[lastEnabled]->play();
				}
			}
			break;
		case State::PAUSED:
			for (auto &&anim : anims_)
			{
				if (anim->state() == State::PAUSED)
				{
					anim->play();
					break;
				}
			}
			break;
		case State::PLAYING:
			break;
	}

	state_ = State::PLAYING;
}

void SequentialAnimationGroup::update(float deltaTime)
{
	int playingIndex = -1;

	if (state_ == IAnimation::State::PLAYING)
	{
		if (shouldWaitDelay(deltaTime) ||
		    (insideSequential() == false && loop_.shouldWaitDelay(deltaTime)))
		{
			return;
		}
		else
		{
			// Check if there is an animation currently in playing state
			for (unsigned int i = 0; i < anims_.size(); i++)
			{
				if (anims_[i]->state() == State::PLAYING)
				{
					playingIndex = i;
					break;
				}
			}
		}
	}

	// Update all enabled animations anyway (always after checking if one is playing)
	for (unsigned int i = 0; i < anims_.size(); i++)
	{
		if (anims_[i]->enabled)
			anims_[i]->update(deltaTime);
		else if (anims_[i]->state() == IAnimation::State::PLAYING)
			anims_[i]->stop();
	}

	if (playingIndex > -1 && state_ == IAnimation::State::PLAYING)
	{
		// Decide the next animation to play if the last playing one has just finished
		if (anims_[playingIndex]->state() == State::STOPPED)
		{
			const Loop::Mode loopMode = loop_.mode();
			// Disable looping if the animation is inside a sequential group
			if (insideSequential())
				loop_.setMode(Loop::Mode::DISABLED);

			if (shouldReverseAnimDirection())
				reverseAnimDirection(*anims_[playingIndex]);

			playingIndex = nextPlayingIndex(playingIndex);

			loop_.setMode(loopMode);

			// The last animation finished and looping is disabled
			if (playingIndex < 0 || playingIndex > anims_.size() - 1)
			{
				resetDelay();
				loop_.resetDelay();
				state_ = State::STOPPED;
			}
			else if (anims_[playingIndex]->state() != State::PLAYING)
			{
				if (shouldReverseAnimDirection())
					reverseAnimDirection(*anims_[playingIndex]);

				anims_[playingIndex]->play();
			}
		}
	}
}

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

int SequentialAnimationGroup::nextPlayingIndex(int playingIndex)
{
	// Search the index of the first animation that is enabled
	int firstEnabled = -1;
	for (unsigned int i = 0; i < anims_.size(); i++)
	{
		if (anims_[i]->enabled)
		{
			firstEnabled = i;
			break;
		}
	}

	// Return an invalid index if there are no enabled animations in the group
	if (firstEnabled < 0)
		return -1;

	// Search the index of the last animation that is enabled
	int lastEnabled = anims_.size() - 1;
	for (int i = anims_.size() - 1; i >= 0; i--)
	{
		if (anims_[i]->enabled)
		{
			lastEnabled = i;
			break;
		}
	}

	do
	{
		switch (loop_.mode())
		{
			case Loop::Mode::DISABLED:
				playingIndex += (loop_.direction() == Loop::Direction::FORWARD) ? 1 : -1;
				break;
			case Loop::Mode::REWIND:
				playingIndex += (loop_.direction() == Loop::Direction::FORWARD) ? 1 : -1;
				if (playingIndex > lastEnabled)
				{
					playingIndex = firstEnabled;
					// Stop all animations to get the initial state
					stopAnimations();
					loop_.justResetNow();
				}
				else if (playingIndex < firstEnabled)
				{
					playingIndex = lastEnabled;
					// Stop all animations to get the initial state
					stopAnimations();
					loop_.justResetNow();
				}
				break;
			case Loop::Mode::PING_PONG:
				if (loop_.isGoingForward())
				{
					playingIndex++;
					if (playingIndex > lastEnabled)
					{
						// Playing again the last animation but reverted
						playingIndex = lastEnabled;
						loop_.justResetNow();
						loop_.toggleForward();
					}
				}
				else
				{
					playingIndex--;
					if (playingIndex < firstEnabled)
					{
						// Playing again the first animation but reverted
						playingIndex = firstEnabled;
						loop_.justResetNow();
						loop_.toggleForward();
					}
				}
				break;
		}
	} while (playingIndex >=0 && playingIndex < anims_.size() && anims_[playingIndex]->enabled == false);

	return playingIndex;
}
