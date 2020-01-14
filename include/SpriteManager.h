#ifndef CLASS_SPRITEMANAGER
#define CLASS_SPRITEMANAGER

#include <nctl/Array.h>

class Sprite;
class Texture;

/// The sprite manager class
class SpriteManager
{
  public:
	SpriteManager();

	inline nctl::Array<nctl::UniquePtr<Texture>> &textures() { return textures_; }
	inline const nctl::Array<nctl::UniquePtr<Texture>> &textures() const { return textures_; }

	inline nctl::Array<nctl::UniquePtr<Sprite>> &sprites() { return sprites_; }
	inline const nctl::Array<nctl::UniquePtr<Sprite>> &sprites() const { return sprites_; }

	void update();

	int textureIndex(const Texture *texture) const;
	int spriteIndex(const Sprite *sprite) const;

  private:
	nctl::Array<nctl::UniquePtr<Texture>> textures_;
	nctl::Array<nctl::UniquePtr<Sprite>> sprites_;
};

#endif
