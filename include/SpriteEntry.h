#ifndef CLASS_SPRITEENTRY
#define CLASS_SPRITEENTRY

#include <nctl/Array.h>
#include <nctl/String.h>
#include <ncine/Colorf.h>

namespace nc = ncine;

class Sprite;
class SpriteGroup;

/// The class that contains information about an entry in the Sprites window
class SpriteEntry
{
  public:
	enum class Type
	{
		GROUP,
		SPRITE
	};

	SpriteEntry(Type type);
	SpriteEntry(Type type, nc::Colorf entryColor);

	inline Type type() const { return type_; }

	inline bool isGroup() const { return type_ == Type::GROUP; }
	inline bool isSprite() const { return type_ == Type::SPRITE; }

	const SpriteGroup *toGroup() const;
	SpriteGroup *toGroup();
	const Sprite *toSprite() const;
	Sprite *toSprite();

	unsigned int spriteId() const;
	void setSpriteId(unsigned int spriteId);

	inline const SpriteGroup *parentGroup() const { return parentGroup_; }
	inline SpriteGroup *parentGroup() { return parentGroup_; }
	inline void setParentGroup(SpriteGroup *parentGroup) { parentGroup_ = parentGroup; }

	inline const nc::Colorf &entryColor() const { return entyrColor_; }
	inline nc::Colorf &entryColor() { return entyrColor_; }

	int indexInParent() const;

  protected:
	const Type type_;
	/// Not used if the entry is a group
	unsigned int spriteId_;
	SpriteGroup *parentGroup_ = nullptr;
	nc::Colorf entyrColor_;
};

class SpriteGroup : public SpriteEntry
{
  public:
	SpriteGroup();
	SpriteGroup(const char *name);
	SpriteGroup(nc::Colorf entryColor, const char *name);

	nctl::UniquePtr<SpriteGroup> clone() const;

	inline const nctl::String &name() const { return name_; }
	inline nctl::String &name() { return name_; }

	inline nctl::Array<nctl::UniquePtr<SpriteEntry>> &children() { return children_; }
	inline const nctl::Array<nctl::UniquePtr<SpriteEntry>> &children() const { return children_; }

  private:
	nctl::String name_;
	nctl::Array<nctl::UniquePtr<SpriteEntry>> children_;
};

#endif
