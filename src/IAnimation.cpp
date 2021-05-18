#include "IAnimation.h"
#include "AnimationGroup.h"

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

IAnimation::IAnimation()
    : state_(State::STOPPED), parent_(nullptr),
      delay_(0.0f), currentDelay_(0.0f)
{}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void IAnimation::stop()
{
	state_ = State::STOPPED;
	currentDelay_ = 0.0f;
}

bool IAnimation::shouldWaitDelay(float deltaTime)
{
	currentDelay_ += deltaTime;
	if (currentDelay_ < delay_)
		return true;
	else
	{
		currentDelay_ = delay_;
		return false;
	}
}

int IAnimation::indexInParent() const
{
	if (parent_ == nullptr)
		return -1;

	int index = -1;
	const nctl::Array<nctl::UniquePtr<IAnimation>> &anims = parent_->anims();
	for (unsigned int i = 0; i < anims.size(); i++)
	{
		if (anims[i].get() == this)
		{
			index = static_cast<int>(i);
			break;
		}
	}

	return index;
}

///////////////////////////////////////////////////////////
// PROTECTED FUNCTIONS
///////////////////////////////////////////////////////////

void IAnimation::cloneTo(IAnimation &other) const
{
	other.name = name;
	other.enabled = enabled;
	// Animation state is not cloned
	other.state_ = State::STOPPED;
	other.parent_ = parent_;
	other.delay_ = delay_;
	other.currentDelay_ = 0.0f;
}
