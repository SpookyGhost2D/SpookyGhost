#ifndef CLASS_SPRITEMANAGER
#define CLASS_SPRITEMANAGER

#include <nctl/Array.h>

class SpriteEntry;
class SpriteGroup;
class Sprite;
class Texture;

/// The sprite manager class
class SpriteManager
{
  public:
	SpriteManager();

	inline nctl::Array<nctl::UniquePtr<Texture>> &textures() { return textures_; }
	inline const nctl::Array<nctl::UniquePtr<Texture>> &textures() const { return textures_; }

	inline SpriteGroup &root() { return *root_; }
	inline const SpriteGroup &root() const { return *root_; }

	nctl::Array<nctl::UniquePtr<SpriteEntry>> &children();
	const nctl::Array<nctl::UniquePtr<SpriteEntry>> &children() const;

	inline nctl::Array<Sprite *> &sprites() { return spritesArray_; }
	inline const nctl::Array<Sprite *> &sprites() const { return spritesArray_; }

	void updateSpritesArray();
	void update();

	int textureIndex(const Texture *texture) const;

	SpriteGroup *addGroup(SpriteEntry *selected);
	Sprite *addSprite(SpriteEntry *selected, Texture *texture);
	void clear();

  private:
	nctl::Array<nctl::UniquePtr<Texture>> textures_;
	nctl::UniquePtr<SpriteGroup> root_;

	nctl::Array<Sprite *> spritesWithoutParent_;
	nctl::Array<Sprite *> spritesArray_;

	void transform(Sprite *sprite);
	void draw(Sprite *sprite);
};

#endif
