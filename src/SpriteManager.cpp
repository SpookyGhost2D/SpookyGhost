#include "SpriteManager.h"
#include "Texture.h"
#include "Sprite.h"

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

SpriteManager::SpriteManager()
    : textures_(4), sprites_(4)
{
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void SpriteManager::update()
{
	for (unsigned int i = 0; i < sprites_.size(); i++)
	{
		sprites_[i]->transform();
		sprites_[i]->updateRender();
		sprites_[i]->render();
		sprites_[i]->resetGrid();
	}
}

void SpriteManager::clear()
{
	sprites_.clear();
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
