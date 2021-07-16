#ifndef CLASS_ANIMATIONMANAGER
#define CLASS_ANIMATIONMANAGER

#include <nctl/UniquePtr.h>
#include "AnimationGroup.h"

class Sprite;
class Script;

/// The animation manager class
class AnimationManager
{
  public:
	AnimationManager();

	inline AnimationGroup &animGroup() { return *animGroup_; }
	inline const AnimationGroup &animGroup() const { return *animGroup_; }

	inline nctl::Array<nctl::UniquePtr<IAnimation>> &anims() { return (*animGroup_).anims(); }
	inline const nctl::Array<nctl::UniquePtr<IAnimation>> &anims() const { return (*animGroup_).anims(); }

	inline IAnimation::State state() const { return animGroup_->state(); }

	inline float speedMultiplier() const { return speedMultiplier_; }
	inline float &speedMultiplier() { return speedMultiplier_; }
	inline void setSpeedMultiplier(float speedMultiplier) { speedMultiplier_ = speedMultiplier; }

	inline void stop() { animGroup_->stop(); }
	inline void pause() { animGroup_->pause(); }
	inline void play() { animGroup_->play(); }

	void update(float deltaTime);
	void clear();

	void removeAnimation(IAnimation *anim);
	void removeSprite(Sprite *sprite);
	void assignGridAnchorToParameters(Sprite *sprite);
	void removeScript(Script *script);
	void reloadScript(Script *script);
	void initScriptsForSprite(Sprite *sprite);
	void overrideSprite(AnimationGroup &animGroup, Sprite *sprite);

  private:
	float speedMultiplier_;
	nctl::UniquePtr<AnimationGroup> animGroup_;
};

#endif
