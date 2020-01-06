#ifndef CLASS_PROPERTYANIMATION
#define CLASS_PROPERTYANIMATION

#include <nctl/String.h>
#include "IAnimation.h"
#include "EasingCurve.h"

/// The animation class
class PropertyAnimation : public IAnimation
{
  public:

	enum class LoopMode
	{
		DISABLED,
		REWIND,
		PING_PONG
	};

	PropertyAnimation();
	PropertyAnimation(EasingCurve::Type curveType, EasingCurve::LoopMode loopMode);

	inline Type type() const override { return Type::PROPERTY; }

	void stop() override;
	void play() override;

	void update(float deltaTime) override;
	inline void reset() override { curve_.reset(); }

	inline EasingCurve &curve() { return curve_; }

	inline const float *property() const { return property_; }
	inline float *property() { return property_; }
	inline void setProperty(float *property) { property_ = property; }
	inline const nctl::String &propertyName() const { return propertyName_; }
	inline void setPropertyName(const nctl::String &name) { propertyName_ = name; }
	//setProperty(void *fun); //TODO: with setter function

  private:
	EasingCurve curve_;

	float *property_;
	nctl::String propertyName_;
};

#endif
