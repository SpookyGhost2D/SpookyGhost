#include "SpriteEntry.h"
#include "Sprite.h" // for `SpriteGroup::clone()`
#include <ncine/Random.h>

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

SpriteEntry::SpriteEntry(Type type)
    : type_(type), parentGroup_(nullptr), entyrColor_(nc::Colorf::White)
{
	entyrColor_.set(nc::random().fastReal(0.0f, 1.0f), nc::random().fastReal(0.0f, 1.0f), nc::random().fastReal(0.0f, 1.0f), 1.0f);
}

SpriteEntry::SpriteEntry(Type type, nc::Colorf entryColor)
    : type_(type), parentGroup_(nullptr), entyrColor_(entryColor)
{
}

SpriteGroup::SpriteGroup()
    : SpriteEntry(Type::GROUP)
{
}

SpriteGroup::SpriteGroup(const char *name)
    : SpriteEntry(Type::GROUP), name_(name)
{
}

SpriteGroup::SpriteGroup(nc::Colorf entryColor, const char *name)
    : SpriteEntry(Type::GROUP, entryColor), name_(name)
{
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

const SpriteGroup *SpriteEntry::toGroup() const
{
	return (type_ == Type::GROUP) ? reinterpret_cast<const SpriteGroup *>(this) : nullptr;
}

SpriteGroup *SpriteEntry::toGroup()
{
	return (type_ == Type::GROUP) ? reinterpret_cast<SpriteGroup *>(this) : nullptr;
}

const Sprite *SpriteEntry::toSprite() const
{
	return (type_ == Type::SPRITE) ? reinterpret_cast<const Sprite *>(this) : nullptr;
}

Sprite *SpriteEntry::toSprite()
{
	return (type_ == Type::SPRITE) ? reinterpret_cast<Sprite *>(this) : nullptr;
}

unsigned int SpriteEntry::spriteId() const
{
	ASSERT(type_ == Type::SPRITE);
	return spriteId_;
}

void SpriteEntry::setSpriteId(unsigned int spriteId)
{
	ASSERT(type_ == Type::SPRITE);
	spriteId_ = spriteId;
}

int SpriteEntry::indexInParent() const
{
	if (parentGroup_ == nullptr)
		return -1;

	int index = -1;
	const nctl::Array<nctl::UniquePtr<SpriteEntry>> &children = parentGroup_->children();
	for (unsigned int i = 0; i < children.size(); i++)
	{
		if (children[i].get() == this)
		{
			index = static_cast<int>(i);
			break;
		}
	}

	return index;
}

nctl::UniquePtr<SpriteGroup> SpriteGroup::clone() const
{
	// Cloned groups have random colors like cloned sprites
	nctl::UniquePtr<SpriteGroup> spriteGroup = nctl::makeUnique<SpriteGroup>(name_.data());
	spriteGroup->setParentGroup(parentGroup_);

	for (unsigned int i = 0; i < children_.size(); i++)
	{
		const SpriteEntry *child = children_[i].get();
		if (child->isGroup())
			spriteGroup->children().pushBack(nctl::move(child->toGroup()->clone()));
		else if (child->isSprite())
			spriteGroup->children().pushBack(nctl::move(child->toSprite()->clone()));
		spriteGroup->children().back()->setParentGroup(spriteGroup.get());
	}

	return spriteGroup;
}
