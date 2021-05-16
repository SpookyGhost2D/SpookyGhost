#include "PropertyAnimation.h"
#include "Sprite.h"
#include "AnimationGroup.h"

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

PropertyAnimation::PropertyAnimation()
    : PropertyAnimation(nullptr)
{
}

PropertyAnimation::PropertyAnimation(Sprite *sprite)
    : CurveAnimation(EasingCurve::Type::LINEAR, Loop::Mode::DISABLED),
      property_(nullptr), propertyName_(64), sprite_(nullptr)
{
	setSprite(sprite);
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

nctl::UniquePtr<IAnimation> PropertyAnimation::clone() const
{
	nctl::UniquePtr<PropertyAnimation> anim = nctl::makeUnique<PropertyAnimation>(sprite_);
	CurveAnimation::cloneTo(*anim);

	anim->property_ = property_;
	anim->propertyName_.assign(propertyName_);

	return nctl::move(anim);
}

void PropertyAnimation::stop()
{
	CurveAnimation::stop();
	if (property_)
		*property_ = curve().value();
}

void PropertyAnimation::update(float deltaTime)
{
	switch (state_)
	{
		case State::STOPPED:
			if (curve().hasInitialValue())
				curve().setTime(curve().initialValue());

			if (parent_->state() != State::PLAYING && isLocked_ && property_)
				*property_ = curve().value();
			break;
		case State::PAUSED:
			if (parent_->state() != State::PLAYING && isLocked_ && property_)
				*property_ = curve().value();
			break;
		case State::PLAYING:
			if (shouldWaitDelay(deltaTime))
				return;
			if (curve_.loop().shouldWaitDelay(deltaTime) == false)
				curve_.next(speed_ * deltaTime);

			const float value = curve_.value();
			if (property_)
				*property_ = value;
			break;
	}

	CurveAnimation::update(deltaTime);
}

namespace Properties {

void assign(PropertyAnimation &anim, Types type)
{
	if (anim.sprite() == nullptr)
		return;

	anim.setProperty(nullptr);
	switch (type)
	{
		case Types::NONE:
			anim.setProperty(nullptr);
			break;
		case Types::POSITION_X:
			anim.setProperty(&anim.sprite()->x);
			break;
		case Types::POSITION_Y:
			anim.setProperty(&anim.sprite()->y);
			break;
		case Types::ROTATION:
			anim.setProperty(&anim.sprite()->rotation);
			break;
		case Types::SCALE_X:
			anim.setProperty(&anim.sprite()->scaleFactor.x);
			break;
		case Types::SCALE_Y:
			anim.setProperty(&anim.sprite()->scaleFactor.y);
			break;
		case Types::ANCHOR_X:
			anim.setProperty(&anim.sprite()->anchorPoint.x);
			break;
		case Types::ANCHOR_Y:
			anim.setProperty(&anim.sprite()->anchorPoint.y);
			break;
		case Types::OPACITY:
			anim.setProperty(&anim.sprite()->color.data()[3]);
			break;
		case Types::COLOR_R:
			anim.setProperty(&anim.sprite()->color.data()[0]);
			break;
		case Types::COLOR_G:
			anim.setProperty(&anim.sprite()->color.data()[1]);
			break;
		case Types::COLOR_B:
			anim.setProperty(&anim.sprite()->color.data()[2]);
			break;
	}
}

void assign(PropertyAnimation &anim, const char *name)
{
	nctl::String string(name);
	for (unsigned int i = 0; i < Count; i++)
	{
		if (string == Strings[i])
		{
			assign(anim, static_cast<Types>(i));
			break;
		}
	}
}

}
