#ifndef CLASS_GRIDANIMATION
#define CLASS_GRIDANIMATION

#include <nctl/Array.h>
#include <ncine/Vector2.h>
#include "CurveAnimation.h"
#include "GridFunctionParameter.h"

class Sprite;
class GridFunction;

/// The grid animation class
class GridAnimation : public CurveAnimation
{
  public:
	GridAnimation();
	GridAnimation(Sprite *sprite);

	nctl::UniquePtr<IAnimation> clone() const override;

	inline Type type() const override { return Type::GRID; }

	void perform() override;

	inline const Sprite *sprite() const { return sprite_; }
	inline Sprite *sprite() { return sprite_; }
	void setSprite(Sprite *sprite);

	inline const GridFunction *function() const { return gridFunction_; }
	void setFunction(const GridFunction *function);

	inline const nctl::Array<GridFunctionParameter> &parameters() const { return params_; }
	inline nctl::Array<GridFunctionParameter> &parameters() { return params_; }

  private:
	Sprite *sprite_;
	const GridFunction *gridFunction_;
	nctl::Array<GridFunctionParameter> params_;
};

#endif
