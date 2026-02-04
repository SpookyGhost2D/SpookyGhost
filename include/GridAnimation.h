#ifndef CLASS_GRIDANIMATION
#define CLASS_GRIDANIMATION

#include <nctl/Array.h>
#include <ncine/Vector2.h>
#include "SpriteAnimation.h"
#include "GridFunctionParameter.h"

class GridFunction;

/// The grid animation class
class GridAnimation : public SpriteAnimation
{
  public:
	GridAnimation();
	explicit GridAnimation(Sprite *sprite);

	nctl::UniquePtr<IAnimation> clone() const override;

	inline Type type() const override { return Type::GRID; }

	void perform() override;

	void setSprite(Sprite *sprite) override;

	inline const GridFunction *function() const { return gridFunction_; }
	void setFunction(const GridFunction *function);

	inline const nctl::Array<GridFunctionParameter> &parameters() const { return params_; }
	inline nctl::Array<GridFunctionParameter> &parameters() { return params_; }

  private:
	const GridFunction *gridFunction_;
	nctl::Array<GridFunctionParameter> params_;
};

#endif
