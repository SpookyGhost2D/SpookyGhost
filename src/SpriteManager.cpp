#include "SpriteManager.h"
#include "Texture.h"
#include "Sprite.h"
#include <ncine/GLBlending.h>

namespace {

void setBlendingFactors(Sprite::BlendingPreset blendingPreset, GLenum &sfactor, GLenum &dfactor)
{
	switch (blendingPreset)
	{
		case Sprite::BlendingPreset::DISABLED:
			sfactor = GL_ONE;
			dfactor = GL_ZERO;
			break;
		case Sprite::BlendingPreset::ALPHA:
			sfactor = GL_SRC_ALPHA;
			dfactor = GL_ONE_MINUS_SRC_ALPHA;
			break;
		case Sprite::BlendingPreset::PREMULTIPLIED_ALPHA:
			sfactor = GL_ONE;
			dfactor = GL_ONE_MINUS_SRC_ALPHA;
			break;
		case Sprite::BlendingPreset::ADDITIVE:
			sfactor = GL_SRC_ALPHA;
			dfactor = GL_ONE;
			break;
		case Sprite::BlendingPreset::MULTIPLY:
			sfactor = GL_DST_COLOR;
			dfactor = GL_ZERO;
			break;
	}
}

void setBlendingFactors(Sprite::BlendingPreset rgbBlendingPreset, Sprite::BlendingPreset alphaBlendingPreset)
{
	GLenum rgbSourceFactor = GL_ONE;
	GLenum rgbDestFactor = GL_ZERO;
	GLenum alphaSourceFactor = GL_ONE;
	GLenum alphaDestFactor = GL_ZERO;

	setBlendingFactors(rgbBlendingPreset, rgbSourceFactor, rgbDestFactor);
	setBlendingFactors(alphaBlendingPreset, alphaSourceFactor, alphaDestFactor);

	nc::GLBlending::setBlendFunc(rgbSourceFactor, rgbDestFactor, alphaSourceFactor, alphaDestFactor);
}

void recursiveLinearizeSprites(SpriteGroup &group, nctl::Array<Sprite *> &sprites, unsigned int &spriteId)
{
	for (unsigned int i = 0; i < group.children().size(); i++)
	{
		SpriteEntry &child = *group.children()[i];
		if (child.isSprite())
		{
			sprites.pushBack(child.toSprite());
			child.setSpriteId(spriteId++);
		}
		else if (child.isGroup())
			recursiveLinearizeSprites(*child.toGroup(), sprites, spriteId);
	}
}

}

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

SpriteManager::SpriteManager()
    : textures_(4), root_(nctl::makeUnique<SpriteGroup>("Root")), spritesWithoutParent_(4), spritesArray_(4)
{
	nc::GLBlending::enable();
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

nctl::Array<nctl::UniquePtr<SpriteEntry>> &SpriteManager::children()
{
	return root_->children();
}

const nctl::Array<nctl::UniquePtr<SpriteEntry>> &SpriteManager::children() const
{
	return root_->children();
}

void SpriteManager::updateSpritesArray()
{
	spritesArray_.clear();
	unsigned int spriteId = 0;
	recursiveLinearizeSprites(*root_, spritesArray_, spriteId);
}

void SpriteManager::update()
{
	spritesWithoutParent_.clear();
	for (unsigned int i = 0; i < spritesArray_.size(); i++)
	{
		if (spritesArray_[i]->parent() == nullptr)
			spritesWithoutParent_.pushBack(spritesArray_[i]);
	}

	for (unsigned int i = 0; i < spritesWithoutParent_.size(); i++)
		transform(spritesWithoutParent_[i]);

	for (unsigned int i = 0; i < spritesArray_.size(); i++)
		draw(spritesArray_[i]);
}

int SpriteManager::textureIndex(const Texture *texture) const
{
	if (texture == nullptr)
		return -1;

	int index = -1;
	for (unsigned int i = 0; i < textures_.size(); i++)
	{
		if (textures_[i].get() == texture)
		{
			index = static_cast<int>(i);
			break;
		}
	}

	return index;
}

SpriteGroup *SpriteManager::addGroup(SpriteEntry *selected)
{
	SpriteGroup *parent = root_.get();
	if (selected && selected->isGroup())
		parent = selected->toGroup();
	else if (selected && selected->isSprite())
		parent = selected->parentGroup();

	nctl::UniquePtr<SpriteGroup> spriteGroup = nctl::makeUnique<SpriteGroup>();
	spriteGroup->setParentGroup(parent);
	parent->children().pushBack(nctl::move(spriteGroup));

	return parent->children().back()->toGroup();
}

Sprite *SpriteManager::addSprite(SpriteEntry *selected, Texture *texture)
{
	SpriteGroup *parent = root_.get();
	if (selected && selected->isGroup())
		parent = selected->toGroup();
	else if (selected && selected->isSprite())
		parent = selected->parentGroup();

	nctl::UniquePtr<Sprite> sprite = nctl::makeUnique<Sprite>(texture);
	sprite->setParentGroup(parent);
	parent->children().pushBack(nctl::move(sprite));

	return parent->children().back()->toSprite();
}

void SpriteManager::clear()
{
	root_->children().clear();
	spritesArray_.clear();
	textures_.clear();
}

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

void SpriteManager::transform(Sprite *sprite)
{
	sprite->transform();

	for (unsigned int i = 0; i < sprite->children().size(); i++)
		transform(sprite->children()[i]);
}

void SpriteManager::draw(Sprite *sprite)
{
	if (sprite->visible == false)
		return;

	sprite->updateRender();
	setBlendingFactors(sprite->rgbBlendingPreset(), sprite->alphaBlendingPreset());
	sprite->render();
	sprite->resetGrid();
}
