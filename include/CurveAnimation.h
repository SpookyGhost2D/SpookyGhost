#ifndef CLASS_CURVEANIMATION
#define CLASS_CURVEANIMATION

#include "IAnimation.h"
#include "EasingCurve.h"

/// The curve animation abstract class
class CurveAnimation : public IAnimation
{
  public:
	CurveAnimation();
	CurveAnimation(EasingCurve::Type curveType, Loop::Mode loopMode);

	/// Returns true if the curve value is applied at every update even if the animation is stopped
	inline bool isLocked() const { return isLocked_; }
	void setLocked(bool locked) { isLocked_ = locked; }

	void stop() override;
	void play() override;

	void update(float deltaTime) override;
	inline void reset() override { curve_.reset(); }

	inline const EasingCurve &curve() const { return curve_; }
	inline EasingCurve &curve() { return curve_; }

	inline float speed() const { return speed_; }
	inline float &speed() { return speed_; }
	inline void setSpeed(float speed) { speed_ = speed; }

  protected:
	bool isLocked_;
	EasingCurve curve_;
	float speed_;

	void cloneTo(CurveAnimation &other) const;
};

#endif
