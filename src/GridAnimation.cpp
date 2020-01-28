#include "GridAnimation.h"
#include "Sprite.h"
#include "GridFunction.h"
#include "GridFunctionLibrary.h"

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

GridAnimation::GridAnimation()
    : GridAnimation(EasingCurve::Type::LINEAR, EasingCurve::LoopMode::DISABLED)
{
}

GridAnimation::GridAnimation(EasingCurve::Type type, EasingCurve::LoopMode loopMode)
    : CurveAnimation(type, loopMode), sprite_(nullptr), gridFunction_(nullptr), params_(4)
{
	setFunction(&GridFunctionLibrary::gridFunctions()[0]);
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

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
			if (sprite_ && sprite_->visible && gridFunction_)
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

	gridFunction_ = function;
}
