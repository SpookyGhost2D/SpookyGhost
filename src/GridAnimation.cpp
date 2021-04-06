#include "GridAnimation.h"
#include "Sprite.h"
#include "GridFunction.h"
#include "GridFunctionLibrary.h"

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

GridAnimation::GridAnimation()
    : GridAnimation(nullptr)
{
}

GridAnimation::GridAnimation(Sprite *sprite)
    : CurveAnimation(EasingCurve::Type::LINEAR, EasingCurve::LoopMode::DISABLED),
      sprite_(nullptr), gridFunction_(nullptr), params_(4)
{
	setSprite(sprite);
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

nctl::UniquePtr<IAnimation> GridAnimation::clone() const
{
	nctl::UniquePtr<GridAnimation> anim = nctl::makeUnique<GridAnimation>(sprite_);

	anim->name.assign(name);
	anim->enabled = enabled;
	// Animation state is not cloned
	anim->setParent(parent_);

	// Always disable locking for cloned animations
	anim->isLocked_ = false;
	anim->curve_ = curve_;
	anim->speed_ = speed_;

	anim->gridFunction_ = gridFunction_;
	anim->params_ = params_;

	return nctl::move(anim);
}

void GridAnimation::stop()
{
	if (state_ == State::STOPPED)
		return;

	CurveAnimation::stop();
	if (sprite_ && gridFunction_)
		gridFunction_->execute(*this);
}

void GridAnimation::update(float deltaTime)
{
	switch (state_)
	{
		case State::STOPPED:
		case State::PAUSED:
			if (isLocked_ && sprite_ && sprite_->visible && gridFunction_)
				gridFunction_->execute(*this);
			break;
		case State::PLAYING:
			curve_.next(speed_ * deltaTime);
			if (sprite_ && sprite_->visible && gridFunction_)
				gridFunction_->execute(*this);
			break;
	}

	CurveAnimation::update(deltaTime);
}

void GridAnimation::setSprite(Sprite *sprite)
{
	if (sprite_)
		sprite_->decrementGridAnimCounter();
	if (sprite)
		sprite->incrementGridAnimCounter();

	sprite_ = sprite;
}

void GridAnimation::setFunction(const GridFunction *function)
{
	if (function)
	{
		if (function->numParameters() != params_.size())
			params_.setSize(function->numParameters());

		for (unsigned int i = 0; i < function->numParameters(); i++)
		{
			const GridFunction::ParameterInfo &paramInfo = function->parameterInfo(i);
			params_[i].value0 = paramInfo.initialValue.value0;
			params_[i].value1 = paramInfo.initialValue.value1;
		}
	}
	else
		params_.clear();

	gridFunction_ = function;
}
