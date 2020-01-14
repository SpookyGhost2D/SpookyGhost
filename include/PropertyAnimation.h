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
	PropertyAnimation(EasingCurve::Type curveType, EasingCurve::LoopMode loopMode);

	inline Type type() const override { return Type::PROPERTY; }

	void stop() override;
	void update(float deltaTime) override;

	inline const Sprite *sprite() const { return sprite_; }
	inline void setSprite(Sprite *sprite) { sprite_ = sprite; }

	inline const float *property() const { return property_; }
	inline float *property() { return property_; }
	inline void setProperty(float *property) { property_ = property; }
	inline const nctl::String &propertyName() const { return propertyName_; }
	inline void setPropertyName(const nctl::String &name) { propertyName_ = name; }
	//setProperty(void *function); //TODO: with setter function

  private:
	float *property_;
	nctl::String propertyName_;
	Sprite *sprite_;
};

#endif
