#ifndef CLASS_ANIMATIONMANAGER
#define CLASS_ANIMATIONMANAGER

#include <nctl/Array.h>
//#include <nctl/UniquePtr.h>
class IAnimation;

/// The animation manager class
class AnimationManager
{
  public:
	AnimationManager();

	inline nctl::Array<nctl::UniquePtr<IAnimation>> &anims() { return anims_; }
	inline const nctl::Array<nctl::UniquePtr<IAnimation>> &anims() const { return anims_; }

	//inline IAnimation &anim() { return *anim_.get(); }
	//inline const IAnimation &anim() const { return *anim_.get(); }

	//inline void setAnim(nctl::UniquePtr<IAnimation> &&anim) { anim_ = nctl::move(anim); }

	void update(float deltaTime);
	void reset();

  private:
	nctl::Array<nctl::UniquePtr<IAnimation>> anims_;
	//nctl::UniquePtr<IAnimation> anim_;
};

#endif
