#include "SequentialAnimationGroup.h"
#include "CurveAnimation.h"

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

SequentialAnimationGroup::SequentialAnimationGroup()
    : loop_(Loop::Mode::DISABLED)
{
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

nctl::UniquePtr<IAnimation> SequentialAnimationGroup::clone() const
{
	nctl::UniquePtr<SequentialAnimationGroup> animGroup = nctl::makeUnique<SequentialAnimationGroup>();
	AnimationGroup::cloneTo(*animGroup);

	animGroup->loop_ = loop_;
	animGroup->loop_.resetDelay();

	return nctl::move(animGroup);
}

void SequentialAnimationGroup::stop()
{
	resetDelay();
	loop_.resetDelay();

	for (auto &&anim : anims_)
	{
		if (anim->state() != State::STOPPED)
		{
			if (shouldReverseAnimDirection())
				reverseAnimDirection(*anim);
			break;
		}
	}

	stopAnimations();
	state_ = State::STOPPED;
}

void SequentialAnimationGroup::pause()
{
	if (state_ != State::PLAYING)
		return;

	for (auto &&anim : anims_)
	{
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
	switch (state_)
	{
		case State::STOPPED:
			// Stop all animations to get the initial state
			stopAnimations();

			if (anims_.isEmpty() == false)
			{
				if (loop_.direction_ == Loop::Direction::FORWARD)
				{
					loop_.forward_ = true;
					anims_.front()->play();
				}
				else
				{
					loop_.forward_ = false;
					if (shouldReverseAnimDirection())
						reverseAnimDirection(*anims_.back());
					anims_.back()->play();
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
		if (shouldWaitDelay(deltaTime) || loop_.shouldWaitDelay(deltaTime))
			return;
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

	// Update all enabled animation anyway (always after checking if one is playing)
	for (unsigned int i = 0; i < anims_.size(); i++)
	{
		if (anims_[i]->enabled)
			anims_[i]->update(deltaTime);
	}

	if (playingIndex > -1 && state_ == IAnimation::State::PLAYING)
	{
		// Decide the next animation to play if the last playing one has just finished
		if (anims_[playingIndex]->state() == State::STOPPED)
		{
			if (shouldReverseAnimDirection())
				reverseAnimDirection(*anims_[playingIndex]);

			switch (loop_.mode_)
			{
				case Loop::Mode::DISABLED:
					playingIndex += (loop_.direction_ == Loop::Direction::FORWARD) ? 1 : -1;
					break;
				case Loop::Mode::REWIND:
					playingIndex += (loop_.direction_ == Loop::Direction::FORWARD) ? 1 : -1;
					if (playingIndex > static_cast<int>(anims_.size() - 1))
					{
						playingIndex = 0;
						// Stop all animations to get the initial state
						stopAnimations();
						loop_.hasJustReset_ = true;
					}
					else if (playingIndex < 0)
					{
						playingIndex = anims_.size() - 1;
						// Stop all animations to get the initial state
						stopAnimations();
						loop_.hasJustReset_ = true;
					}
					break;
				case Loop::Mode::PING_PONG:
					if (loop_.forward_)
					{
						playingIndex++;
						if (playingIndex >= anims_.size())
						{
							// Playing again the last animation but reverted
							playingIndex = anims_.size() - 1;
							loop_.hasJustReset_ = true;
							loop_.forward_ = !loop_.forward_;
						}
					}
					else
					{
						playingIndex--;
						if (playingIndex < 0)
						{
							// Playing again the first animation but reverted
							playingIndex = 0;
							loop_.hasJustReset_ = true;
							loop_.forward_ = !loop_.forward_;
						}
					}
					break;
			}

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

void SequentialAnimationGroup::stopAnimations()
{
	if (loop_.direction_ == Loop::Direction::FORWARD)
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

bool SequentialAnimationGroup::shouldReverseAnimDirection()
{
	return ((loop_.mode_ != Loop::Mode::PING_PONG && loop_.direction_ == Loop::Direction::BACKWARD) ||
	        (loop_.mode_ == Loop::Mode::PING_PONG && loop_.isGoingForward() == false));
}

void SequentialAnimationGroup::reverseAnimDirection(IAnimation &anim)
{
	if (anim.type() != Type::PARALLEL_GROUP)
	{
		if (anim.type() == Type::SEQUENTIAL_GROUP)
			static_cast<SequentialAnimationGroup &>(anim).loop().reverseDirection();
		else
			static_cast<CurveAnimation &>(anim).curve().loop().reverseDirection();
	}
}
