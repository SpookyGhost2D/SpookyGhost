#include "SpriteManager.h"
#include "Texture.h"
#include "Sprite.h"
#include <ncine/GLBlending.h>

namespace {

void setBlendingFactors(Sprite::BlendingPreset blendingPreset)
{
	switch (blendingPreset)
	{
		case Sprite::BlendingPreset::DISABLED:
			nc::GLBlending::blendFunc(GL_ONE, GL_ZERO);
			break;
		case Sprite::BlendingPreset::ALPHA:
			nc::GLBlending::blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		case Sprite::BlendingPreset::PREMULTIPLIED_ALPHA:
			nc::GLBlending::blendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			break;
		case Sprite::BlendingPreset::ADDITIVE:
			nc::GLBlending::blendFunc(GL_SRC_ALPHA, GL_ONE);
			break;
		case Sprite::BlendingPreset::MULTIPLY:
			nc::GLBlending::blendFunc(GL_DST_COLOR, GL_ZERO);
			break;
	}
}

}

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

SpriteManager::SpriteManager()
    : textures_(4), sprites_(4)
{
	nc::GLBlending::enable();
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void SpriteManager::update()
{
	for (unsigned int i = 0; i < sprites_.size(); i++)
	{
		if (sprites_[i]->visible == false)
			continue;
		sprites_[i]->transform();
		sprites_[i]->updateRender();
		setBlendingFactors(sprites_[i]->blendingPreset());
		sprites_[i]->render();
		sprites_[i]->resetGrid();
	}
}

// TODO: Get rid of this function
int SpriteManager::textureIndex(const Texture *texture) const
{
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
