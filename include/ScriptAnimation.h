#ifndef CLASS_SCRIPTANIMATION
#define CLASS_SCRIPTANIMATION

#include "SpriteAnimation.h"

class Script;

/// The script animation class
class ScriptAnimation : public SpriteAnimation
{
  public:
	ScriptAnimation();
	ScriptAnimation(Sprite *sprite, Script *script);

	nctl::UniquePtr<IAnimation> clone() const override;

	inline Type type() const override { return Type::SCRIPT; }

	void play() override;
	void perform() override;

	void setSprite(Sprite *sprite) override;

	inline const Script *script() const { return script_; }
	inline Script *script() { return script_; }
	void setScript(Script *script);

  private:
	Script *script_;

	bool runScript(const char *functionName, float value);
};

#endif
