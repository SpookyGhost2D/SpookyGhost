#include "GridAnimation.h"
#include "Sprite.h"
#include "GridFunction.h"
#include "GridFunctionLibrary.h"
#include "AnimationGroup.h"

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

GridAnimation::GridAnimation()
    : GridAnimation(nullptr)
{
}

GridAnimation::GridAnimation(Sprite *sprite)
    : CurveAnimation(EasingCurve::Type::LINEAR, Loop::Mode::DISABLED),
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
	CurveAnimation::cloneTo(*anim);

	anim->gridFunction_ = gridFunction_;
	anim->params_ = params_;

	return nctl::move(anim);
}

void GridAnimation::perform()
{
	if (sprite_ && sprite_->visible && gridFunction_)
		gridFunction_->execute(*this);
}

void GridAnimation::setSprite(Sprite *sprite)
{
	if (sprite_ != sprite)
	{
		if (sprite_)
			sprite_->decrementGridAnimCounter();
		if (sprite)
			sprite->incrementGridAnimCounter();

		sprite_ = sprite;
	}
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
