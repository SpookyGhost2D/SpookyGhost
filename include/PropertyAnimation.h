#ifndef CLASS_PROPERTYANIMATION
#define CLASS_PROPERTYANIMATION

#include <nctl/String.h>
#include "CurveAnimation.h"

class Sprite;

/// The property animation class
class PropertyAnimation : public CurveAnimation
{
  public:
	PropertyAnimation();
	PropertyAnimation(Sprite *sprite);

	inline Type type() const override { return Type::PROPERTY; }

	void stop() override;
	void update(float deltaTime) override;

	inline const Sprite *sprite() const { return sprite_; }
	inline Sprite *sprite() { return sprite_; }
	inline void setSprite(Sprite *sprite) { sprite_ = sprite; }

	inline const float *property() const { return property_; }
	inline float *property() { return property_; }
	inline void setProperty(float *property) { property_ = property; }
	inline const nctl::String &propertyName() const { return propertyName_; }
	inline void setPropertyName(const nctl::String &name) { propertyName_ = name; }

  private:
	float *property_;
	nctl::String propertyName_;
	Sprite *sprite_;
};

namespace Properties {

const unsigned int Count = 12;
static const char *Strings[Count] = { "None", "Position X", "Position Y", "Rotation", "Scale X", "Scale Y", "AnchorPoint X", "AnchorPoint Y", "Opacity", "Red Channel", "Green Channel", "Blue Channel" };

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

void assign(PropertyAnimation &anim, Types type);
void assign(PropertyAnimation &anim, const char *name);

}

#endif
