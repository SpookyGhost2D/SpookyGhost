#ifndef CLASS_PROPERTYANIMATION
#define CLASS_PROPERTYANIMATION

#include <nctl/String.h>
#include "CurveAnimation.h"

class Sprite;

namespace Properties {

const unsigned int Count = 12;
static const char *Strings[Count] = { "None", "Position X", "Position Y", "Rotation",
	                                  "Scale X", "Scale Y", "AnchorPoint X", "AnchorPoint Y",
	                                  "Opacity", "Red Channel", "Green Channel", "Blue Channel" };

enum Types
{
	NONE,
	POSITION_X,
	POSITION_Y,
	ROTATION,
	SCALE_X,
	SCALE_Y,
	ANCHOR_X,
	ANCHOR_Y,
	OPACITY,
	COLOR_R,
	COLOR_G,
	COLOR_B
};

}

/// The property animation class
class PropertyAnimation : public CurveAnimation
{
  public:
	PropertyAnimation();
	PropertyAnimation(Sprite *sprite);

	nctl::UniquePtr<IAnimation> clone() const override;

	inline Type type() const override { return Type::PROPERTY; }

	void perform() override;

	inline Properties::Types propertyType() const { return propertyType_; }
	const char *propertyName() const;
	void setProperty(Properties::Types propertyType);
	void setProperty(const char *name);

	inline const Sprite *sprite() const { return sprite_; }
	inline Sprite *sprite() { return sprite_; }
	void setSprite(Sprite *sprite);

	inline const float *property() const { return property_; }

  private:
	Properties::Types propertyType_;
	float *property_;
	Sprite *sprite_;
};

#endif
