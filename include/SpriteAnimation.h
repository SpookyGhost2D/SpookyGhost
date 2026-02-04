#ifndef CLASS_SPRITEANIMATION
#define CLASS_SPRITEANIMATION

#include "CurveAnimation.h"

class Sprite;

/// The base class for sprite based animations
class SpriteAnimation : public CurveAnimation
{
  public:
	SpriteAnimation();
	explicit SpriteAnimation(Sprite *sprite);

	inline const Sprite *sprite() const { return sprite_; }
	inline Sprite *sprite() { return sprite_; }
	virtual void setSprite(Sprite *sprite);

  protected:
	Sprite *sprite_;

	void cloneTo(SpriteAnimation &other) const;
};

#endif
