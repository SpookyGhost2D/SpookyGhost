#include "SequentialAnimationGroup.h"

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

SequentialAnimationGroup::SequentialAnimationGroup()
    : direction_(Direction::FORWARD), loopMode_(LoopMode::DISABLED), forward_(true)
{
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void SequentialAnimationGroup::stop()
{
	if (direction_ == Direction::FORWARD)
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
			if (direction_ == Direction::FORWARD)
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
		anims_[playingIndex]->update(deltaTime);

		// Decide the next animation to play as the current one has just finished
		if (anims_[playingIndex]->state() == State::STOPPED)
		{
			switch (loopMode_)
			{
				case LoopMode::DISABLED:
					playingIndex += (direction_ == Direction::FORWARD) ? 1 : -1;
					break;
				case LoopMode::REWIND:
					playingIndex += (direction_ == Direction::FORWARD) ? 1 : -1;
					if (playingIndex > static_cast<int>(anims_.size() - 1))
						playingIndex = 0;
					else if (playingIndex < 0)
						playingIndex = anims_.size() - 1;
					break;
				case LoopMode::PING_PONG:
					if (forward_)
					{
						playingIndex++;
						if (playingIndex >= anims_.size())
						{
							playingIndex = anims_.size() > 1 ? anims_.size() - 2 : anims_.size() - 1;
							forward_ = !forward_;
						}
					}
					else
					{
						playingIndex--;
						if (playingIndex < 0)
						{
							playingIndex = anims_.size() > 1 ? 1 : 0;
							forward_ = !forward_;
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
