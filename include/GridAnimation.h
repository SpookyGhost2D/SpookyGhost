#ifndef CLASS_GRIDANIMATION
#define CLASS_GRIDANIMATION

#include "IAnimation.h"
#include "EasingCurve.h"

class Sprite;

/// The grid animation class
class GridAnimation : public IAnimation
{
  public:

	enum class LoopMode
	{
		DISABLED,
		REWIND,
		PING_PONG
	};

	enum class AnimationType
	{
		WOBBLE
	};

	GridAnimation();
	GridAnimation(EasingCurve::Type curveType, EasingCurve::LoopMode loopMode);

	inline Type type() const override { return Type::GRID; }

	void stop() override;
	void play() override;

	void update(float deltaTime) override;
	inline void reset() override { curve_.reset(); }

	inline EasingCurve &curve() { return curve_; }

	inline float speed() const { return speed_; }
	inline float &speed() { return speed_; }
	inline void setSpeed(float speed) { speed_ = speed; }

	inline void setSprite(Sprite *sprite) { sprite_ = sprite; }

	inline AnimationType gridAnimationType() const { return type_; }
	inline void setGridAnimationType(AnimationType type) { type_ = type; }

  private:
	EasingCurve curve_;
	float speed_;
	AnimationType type_;
	Sprite *sprite_;

	void deform(float value);
};

#endif
