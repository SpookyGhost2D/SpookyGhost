#ifndef CLASS_GRIDANIMATION
#define CLASS_GRIDANIMATION

#include "CurveAnimation.h"

class Sprite;

/// The grid animation class
class GridAnimation : public CurveAnimation
{
  public:
	enum class AnimationType
	{
		WOBBLE_X,
		WOBBLE_Y,
		SKEW_X,
		SKEW_Y,
		ZOOM
	};

	GridAnimation();
	GridAnimation(EasingCurve::Type curveType, EasingCurve::LoopMode loopMode);

	inline Type type() const override { return Type::GRID; }

	void stop() override;
	void update(float deltaTime) override;

	inline const Sprite *sprite() const { return sprite_; }
	inline void setSprite(Sprite *sprite) { sprite_ = sprite; }

	inline AnimationType gridAnimationType() const { return type_; }
	inline void setGridAnimationType(AnimationType type) { type_ = type; }

  private:
	AnimationType type_;
	Sprite *sprite_;

	void deform(float value);
};

#endif
