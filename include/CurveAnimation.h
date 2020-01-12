#ifndef CLASS_CURVEANIMATION
#define CLASS_CURVEANIMATION

#include "IAnimation.h"
#include "EasingCurve.h"

/// The curve animation abstract class
class CurveAnimation : public IAnimation
{
  public:
	enum class LoopMode
	{
		DISABLED,
		REWIND,
		PING_PONG
	};

	CurveAnimation();
	CurveAnimation(EasingCurve::Type curveType, EasingCurve::LoopMode loopMode);

	void stop() override;
	void play() override;

	void update(float deltaTime) override;
	inline void reset() override { curve_.reset(); }

	inline EasingCurve &curve() { return curve_; }

	inline float speed() const { return speed_; }
	inline float &speed() { return speed_; }
	inline void setSpeed(float speed) { speed_ = speed; }

  protected:
	EasingCurve curve_;
	float speed_;
};

#endif
