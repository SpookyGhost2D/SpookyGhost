#include <ncine/common_constants.h>
#include "LoopComponent.h"

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

LoopComponent::LoopComponent(Loop::Mode mode, Loop::Direction direction)
    : mode_(mode), direction_(direction),
      forward_(true), hasJustReset_(false),
      delay_(0.0f), currentDelay_(0.0f), waitDelay_(false)
{
}

LoopComponent::LoopComponent(Loop::Mode mode)
    : LoopComponent(mode, Loop::Direction::FORWARD)
{
}

LoopComponent::LoopComponent()
    : LoopComponent(Loop::Mode::DISABLED, Loop::Direction::FORWARD)
{
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void LoopComponent::reverseDirection()
{
	if (direction_ == Loop::Direction::FORWARD)
		direction_ = Loop::Direction::BACKWARD;
	else
		direction_ = Loop::Direction::FORWARD;
}

void LoopComponent::resetDelay()
{
	hasJustReset_ = false;
	currentDelay_ = 0.0f;
	waitDelay_ = false;
}

bool LoopComponent::shouldWaitDelay(float deltaTime)
{
	if (waitDelay_)
	{
		currentDelay_ += deltaTime;
		if (currentDelay_ >= delay_)
		{
			currentDelay_ = 0.0f;
			waitDelay_ = false;
		}
		else
			return true;
	}

	if (waitDelay_ == false && hasJustReset_)
	{
		waitDelay_ = true;
		hasJustReset_ = false;
	}

	return false;
}
