#include "AnimationManager.h"
#include "ParallelAnimationGroup.h"
#include "PropertyAnimation.h"
#include "GridAnimation.h"

namespace {

void recursiveRemoveSprite(AnimationGroup &animGroup, Sprite *sprite)
{
	// Deleting backwards without iterators
	for (int i = animGroup.anims().size() - 1; i >= 0; i--)
	{
		IAnimation &anim = *animGroup.anims()[i];

		switch (anim.type())
		{
			case IAnimation::Type::PARALLEL_GROUP:
			case IAnimation::Type::SEQUENTIAL_GROUP:
			{
				AnimationGroup &innerGroup = static_cast<AnimationGroup &>(anim);
				recursiveRemoveSprite(innerGroup, sprite);
				break;
			}
			case IAnimation::Type::PROPERTY:
			{
				PropertyAnimation &propertyAnim = static_cast<PropertyAnimation &>(anim);
				if (propertyAnim.sprite() == sprite)
				{
					propertyAnim.setProperty(nullptr);
					propertyAnim.stop();
					animGroup.anims().removeAt(i);
				}
				break;
			}
			case IAnimation::Type::GRID:
			{
				GridAnimation &gridAnim = static_cast<GridAnimation &>(anim);
				if (gridAnim.sprite() == sprite)
				{
					gridAnim.setSprite(nullptr);
					gridAnim.stop();
					animGroup.anims().removeAt(i);
				}
				break;
			}
		}
	}
}

}

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

AnimationManager::AnimationManager()
    : animGroup_(nctl::makeUnique<ParallelAnimationGroup>())
{
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void AnimationManager::update(float deltaTime)
{
	animGroup_->update(deltaTime);
}

void AnimationManager::reset()
{
	animGroup_->reset();
}

void AnimationManager::clear()
{
	animGroup_->anims().clear();
}

void AnimationManager::removeSprite(Sprite *sprite)
{
	if (sprite == nullptr)
		return;

	recursiveRemoveSprite(*animGroup_, sprite);
}
