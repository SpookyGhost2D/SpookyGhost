#include "PropertyAnimation.h"
#include "Sprite.h"

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

PropertyAnimation::PropertyAnimation()
    : PropertyAnimation(nullptr)
{
}

PropertyAnimation::PropertyAnimation(Sprite *sprite)
    : CurveAnimation(EasingCurve::Type::LINEAR, EasingCurve::LoopMode::DISABLED),
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

	anim->name.assign(name);
	// Animation state is not cloned
	anim->setParent(parent_);

	// Always disable locking for cloned animations
	anim->isLocked_ = false;
	anim->curve_ = curve_;
	anim->speed_ = speed_;

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
		case State::PAUSED:
			if (isLocked_ && property_)
				*property_ = curve().value();
			break;
		case State::PLAYING:
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
