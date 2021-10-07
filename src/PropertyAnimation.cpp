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
      propertyType_(Properties::Types::NONE), property_(nullptr), sprite_(sprite)
{
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

nctl::UniquePtr<IAnimation> PropertyAnimation::clone() const
{
	nctl::UniquePtr<PropertyAnimation> anim = nctl::makeUnique<PropertyAnimation>(sprite_);
	CurveAnimation::cloneTo(*anim);

	anim->setSprite(sprite_);
	anim->setProperty(propertyType_);

	return nctl::move(anim);
}

void PropertyAnimation::perform()
{
	if (property_)
		*property_ = curve_.value();
}

const char *PropertyAnimation::propertyName() const
{
	const int index = static_cast<int>(propertyType_);
	return Properties::Strings[index];
}

void PropertyAnimation::setProperty(Properties::Types propertyType)
{
	if (sprite_ == nullptr)
	{
		propertyType_ = Properties::Types::NONE;
		property_ = nullptr;
		return;
	}

	propertyType_ = propertyType;
	switch (propertyType)
	{
		case Properties::Types::NONE:
			property_ = nullptr;
			break;
		case Properties::Types::POSITION_X:
			property_ = &sprite_->x;
			break;
		case Properties::Types::POSITION_Y:
			property_ = &sprite_->y;
			break;
		case Properties::Types::ROTATION:
			property_ = &sprite_->rotation;
			break;
		case Properties::Types::SCALE_X:
			property_ = &sprite_->scaleFactor.x;
			break;
		case Properties::Types::SCALE_Y:
			property_ = &sprite_->scaleFactor.y;
			break;
		case Properties::Types::ANCHOR_X:
			property_ = &sprite_->anchorPoint.x;
			break;
		case Properties::Types::ANCHOR_Y:
			property_ = &sprite_->anchorPoint.y;
			break;
		case Properties::Types::OPACITY:
			property_ = &sprite_->color.data()[3];
			break;
		case Properties::Types::COLOR_R:
			property_ = &sprite_->color.data()[0];
			break;
		case Properties::Types::COLOR_G:
			property_ = &sprite_->color.data()[1];
			break;
		case Properties::Types::COLOR_B:
			property_ = &sprite_->color.data()[2];
			break;
	}
}

void PropertyAnimation::setProperty(const char *name)
{
	if (name == nullptr)
		setProperty(Properties::Types::NONE);
	else
	{
		nctl::String string(name);
		for (unsigned int i = 0; i < Properties::Count; i++)
		{
			if (string == Properties::Strings[i])
			{
				setProperty(static_cast<Properties::Types>(i));
				break;
			}
		}
	}
}

void PropertyAnimation::setSprite(Sprite *sprite)
{
	if (sprite_ != sprite)
	{
		sprite_ = sprite;
		setProperty(propertyType_);
	}
}
