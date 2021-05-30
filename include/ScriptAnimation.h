#ifndef CLASS_SCRIPTANIMATION
#define CLASS_SCRIPTANIMATION

#include "CurveAnimation.h"

class Sprite;
class Script;

/// The script animation class
class ScriptAnimation : public CurveAnimation
{
  public:
	ScriptAnimation();
	ScriptAnimation(Sprite *sprite, Script *script);

	nctl::UniquePtr<IAnimation> clone() const override;

	inline Type type() const override { return Type::SCRIPT; }

	void play() override;
	void perform() override;

	inline const Sprite *sprite() const { return sprite_; }
	inline Sprite *sprite() { return sprite_; }
	void setSprite(Sprite *sprite);

	inline const Script *script() const { return script_; }
	inline Script *script() { return script_; }
	void setScript(Script *script);

  private:
	Sprite *sprite_;
	Script *script_;

	bool runScript(const char *functionName, float value);
};

#endif
