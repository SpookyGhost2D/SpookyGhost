#include "SequentialAnimationGroup.h"

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

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
			anims_[0]->play();
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
	bool playNext = false;
	bool allStopped = true;
	for(unsigned int i = 0; i < anims_.size(); i++)
	{
		IAnimation *anim = anims_[i].get();
		bool wasPlaying = false;

		if (playNext)
		{
			anim->play();
			playNext = false;
		}
		if (anim->state() == State::PLAYING)
			wasPlaying = true;

		anim->update(deltaTime);

		if (anim->state() == State::STOPPED && wasPlaying)
			playNext = true;

		if (anim->state() != State::STOPPED)
			allStopped = false;
	}
	if (allStopped)
		state_ = State::STOPPED;
}

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////
