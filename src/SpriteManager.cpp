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

	nc::GLBlending::blendFunc(rgbSourceFactor, rgbDestFactor, alphaSourceFactor, alphaDestFactor);
}

}

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

SpriteManager::SpriteManager()
    : textures_(4), sprites_(4), spritesWithoutParent_(4)
{
	nc::GLBlending::enable();
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void SpriteManager::update()
{
	spritesWithoutParent_.clear();
	for (unsigned int i = 0; i < sprites_.size(); i++)
	{
		if (sprites_[i]->parent() == nullptr)
			spritesWithoutParent_.pushBack(sprites_[i].get());
	}

	for (unsigned int i = 0; i < spritesWithoutParent_.size(); i++)
		transform(spritesWithoutParent_[i]);

	for (unsigned int i = 0; i < sprites_.size(); i++)
		draw(sprites_[i].get());
}

// TODO: Get rid of this function
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

// TODO: Get rid of this function
int SpriteManager::spriteIndex(const Sprite *sprite) const
{
	if (sprite == nullptr)
		return -1;

	int index = -1;
	for (unsigned int i = 0; i < sprites_.size(); i++)
	{
		if (sprites_[i].get() == sprite)
		{
			index = static_cast<int>(i);
			break;
		}
	}

	return index;
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
