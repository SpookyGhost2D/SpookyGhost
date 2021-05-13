#include "SequentialAnimationGroup.h"

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

	return nctl::move(animGroup);
}

void SequentialAnimationGroup::stop()
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
			anim->stop();
	}

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
	if (state_ == State::PLAYING)
		return;

	if (state_ == State::STOPPED)
	{
		if (anims_.isEmpty() == false)
		{
			if (loop_.direction_ == Loop::Direction::FORWARD)
				anims_.front()->play();
			else
				anims_.back()->play();
		}
	}
	else if (state_ == State::PAUSED)
	{
		for (auto &&anim : anims_)
		{
			if (anim->state() == State::PAUSED)
			{
				anim->play();
				break;
			}
		}
	}

	state_ = State::PLAYING;
}

void SequentialAnimationGroup::update(float deltaTime)
{
	int playingIndex = -1;
	for (unsigned int i = 0; i < anims_.size(); i++)
	{
		if (anims_[i]->state() == State::PLAYING)
		{
			playingIndex = i;
			break;
		}
	}

	if (playingIndex < 0)
		state_ = State::STOPPED;
	else
	{
		if (anims_[playingIndex]->enabled)
			anims_[playingIndex]->update(deltaTime);
		else
			anims_[playingIndex]->stop();

		// Decide the next animation to play as the current one has just finished
		if (anims_[playingIndex]->state() == State::STOPPED)
		{
			switch (loop_.mode_)
			{
				case Loop::Mode::DISABLED:
					playingIndex += (loop_.direction_ == Loop::Direction::FORWARD) ? 1 : -1;
					break;
				case Loop::Mode::REWIND:
					playingIndex += (loop_.direction_ == Loop::Direction::FORWARD) ? 1 : -1;
					if (playingIndex > static_cast<int>(anims_.size() - 1))
						playingIndex = 0;
					else if (playingIndex < 0)
						playingIndex = anims_.size() - 1;
					break;
				case Loop::Mode::PING_PONG:
					if (loop_.forward_)
					{
						playingIndex++;
						if (playingIndex >= anims_.size())
						{
							playingIndex = anims_.size() > 1 ? anims_.size() - 2 : anims_.size() - 1;
							loop_.forward_ = !loop_.forward_;
						}
					}
					else
					{
						playingIndex--;
						if (playingIndex < 0)
						{
							playingIndex = anims_.size() > 1 ? 1 : 0;
							loop_.forward_ = !loop_.forward_;
						}
					}
					break;
			}
		}
		if (playingIndex < 0 || playingIndex > anims_.size() - 1)
			state_ = State::STOPPED;
		else if (state_ == State::PLAYING)
		{
			// Play the next animation in the group only if the whole group was in playing state.
			// This allows to select and play just a single child animation in the group.
			anims_[playingIndex]->play();
		}
	}
}
