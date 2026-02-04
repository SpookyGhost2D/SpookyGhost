#include "SpriteAnimation.h"
#include "Sprite.h"

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

SpriteAnimation::SpriteAnimation()
    : SpriteAnimation(nullptr)
{
}

SpriteAnimation::SpriteAnimation(Sprite *sprite)
    : CurveAnimation(EasingCurve::Type::LINEAR, Loop::Mode::DISABLED),
      sprite_(sprite)
{
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void SpriteAnimation::setSprite(Sprite *sprite)
{
	sprite_ = sprite;
}

///////////////////////////////////////////////////////////
// PROTECTED FUNCTIONS
///////////////////////////////////////////////////////////

void SpriteAnimation::cloneTo(SpriteAnimation &other) const
{
	CurveAnimation::cloneTo(other);
	other.setSprite(sprite_);
}
