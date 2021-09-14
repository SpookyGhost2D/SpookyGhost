#include "AnimationManager.h"
#include "ParallelAnimationGroup.h"
#include "PropertyAnimation.h"
#include "GridAnimation.h"
#include "GridFunction.h"
#include "ScriptAnimation.h"
#include "Sprite.h"
#include "Script.h"

namespace {

void resetAnimation(IAnimation &anim)
{
	switch (anim.type())
	{
		case IAnimation::Type::PARALLEL_GROUP:
		case IAnimation::Type::SEQUENTIAL_GROUP:
			break;
		case IAnimation::Type::PROPERTY:
		{
			PropertyAnimation &propertyAnim = static_cast<PropertyAnimation &>(anim);
			propertyAnim.setProperty(Properties::Types::NONE);
			propertyAnim.setSprite(nullptr);
			propertyAnim.stop();
			break;
		}
		case IAnimation::Type::GRID:
		{
			GridAnimation &gridAnim = static_cast<GridAnimation &>(anim);
			gridAnim.setSprite(nullptr);
			gridAnim.stop();
			break;
		}
		case IAnimation::Type::SCRIPT:
		{
			ScriptAnimation &scriptAnim = static_cast<ScriptAnimation &>(anim);
			scriptAnim.setSprite(nullptr);
			scriptAnim.stop();
			break;
		}
	}
}

void recursiveRemoveAnimation(IAnimation &anim)
{
	if (anim.isGroup())
	{
		AnimationGroup &animGroup = static_cast<AnimationGroup &>(anim);
		for (unsigned int i = 0; i < animGroup.anims().size(); i++)
			resetAnimation(*animGroup.anims()[i]);
		animGroup.anims().clear();
	}
	else
		resetAnimation(anim);

	if (anim.parent() != nullptr)
	{
		nctl::Array<nctl::UniquePtr<IAnimation>> &anims = anim.parent()->anims();
		for (unsigned int i = 0; i < anims.size(); i++)
		{
			if (anims[i].get() == &anim)
			{
				anims.removeAt(i);
				break;
			}
		}
	}
}

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
					propertyAnim.setProperty(Properties::Types::NONE);
					propertyAnim.setSprite(nullptr);
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
			case IAnimation::Type::SCRIPT:
			{
				ScriptAnimation &scriptAnim = static_cast<ScriptAnimation &>(anim);
				if (scriptAnim.sprite() == sprite)
				{
					scriptAnim.setSprite(nullptr);
					scriptAnim.stop();
					animGroup.anims().removeAt(i);
				}
				break;
			}
		}
	}
}

void recursiveAssignGridAnchorToParameters(AnimationGroup &animGroup, Sprite *sprite)
{
	for (unsigned int i = 0; i < animGroup.anims().size(); i++)
	{
		IAnimation &anim = *animGroup.anims()[i];

		switch (anim.type())
		{
			case IAnimation::Type::PARALLEL_GROUP:
			case IAnimation::Type::SEQUENTIAL_GROUP:
			{
				AnimationGroup &innerGroup = static_cast<AnimationGroup &>(anim);
				recursiveAssignGridAnchorToParameters(innerGroup, sprite);
				break;
			}
			case IAnimation::Type::PROPERTY:
			case IAnimation::Type::SCRIPT:
				break;
			case IAnimation::Type::GRID:
			{
				GridAnimation &gridAnim = static_cast<GridAnimation &>(anim);
				if (gridAnim.sprite() == sprite && gridAnim.function() != nullptr)
				{
					const GridFunction &function = *gridAnim.function();
					for (unsigned int paramIndex = 0; paramIndex < function.numParameters(); paramIndex++)
					{
						if (function.parameterInfo(paramIndex).anchorType == GridFunction::AnchorType::X)
							gridAnim.parameters()[paramIndex].value0 = gridAnim.sprite()->gridAnchorPoint.x;
						else if (function.parameterInfo(paramIndex).anchorType == GridFunction::AnchorType::Y)
							gridAnim.parameters()[paramIndex].value0 = gridAnim.sprite()->gridAnchorPoint.y;
						else if (function.parameterInfo(paramIndex).anchorType == GridFunction::AnchorType::XY)
						{
							gridAnim.parameters()[paramIndex].value0 = gridAnim.sprite()->gridAnchorPoint.x;
							gridAnim.parameters()[paramIndex].value1 = gridAnim.sprite()->gridAnchorPoint.y;
						}
					}
				}
				break;
			}
		}
	}
}

void recursiveRemoveScript(AnimationGroup &animGroup, Script *script)
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
				recursiveRemoveScript(innerGroup, script);
				break;
			}
			case IAnimation::Type::PROPERTY:
			case IAnimation::Type::GRID:
				break;
			case IAnimation::Type::SCRIPT:
			{
				ScriptAnimation &scriptAnim = static_cast<ScriptAnimation &>(anim);
				if (scriptAnim.script() == script)
				{
					scriptAnim.setSprite(nullptr);
					scriptAnim.setScript(nullptr);
					scriptAnim.stop();
					animGroup.anims().removeAt(i);
				}
				break;
			}
		}
	}
}

void recursiveReloadScript(AnimationGroup &animGroup, Script *script)
{
	for (unsigned int i = 0; i < animGroup.anims().size(); i++)
	{
		IAnimation &anim = *animGroup.anims()[i];

		switch (anim.type())
		{
			case IAnimation::Type::PARALLEL_GROUP:
			case IAnimation::Type::SEQUENTIAL_GROUP:
			{
				AnimationGroup &innerGroup = static_cast<AnimationGroup &>(anim);
				recursiveReloadScript(innerGroup, script);
				break;
			}
			case IAnimation::Type::PROPERTY:
			case IAnimation::Type::GRID:
				break;
			case IAnimation::Type::SCRIPT:
			{
				ScriptAnimation &scriptAnim = static_cast<ScriptAnimation &>(anim);
				if (scriptAnim.script() == script)
				{
					const IAnimation::State prevState = scriptAnim.state();
					// Call `play()` again to run the Lua init function
					scriptAnim.play();
					if (prevState != IAnimation::State::PLAYING)
						scriptAnim.stop();
				}
				break;
			}
		}
	}
}

void recursiveInitScriptsForSprite(AnimationGroup &animGroup, Sprite *sprite)
{
	for (unsigned int i = 0; i < animGroup.anims().size(); i++)
	{
		IAnimation &anim = *animGroup.anims()[i];

		switch (anim.type())
		{
			case IAnimation::Type::PARALLEL_GROUP:
			case IAnimation::Type::SEQUENTIAL_GROUP:
			case IAnimation::Type::PROPERTY:
			case IAnimation::Type::GRID:
				break;
			case IAnimation::Type::SCRIPT:
			{
				ScriptAnimation &scriptAnim = static_cast<ScriptAnimation &>(anim);
				if (scriptAnim.sprite() == sprite)
				{
					const IAnimation::State prevState = scriptAnim.state();
					// Call `play()` again to run the Lua init function
					scriptAnim.play();
					if (prevState != IAnimation::State::PLAYING)
						scriptAnim.stop();
				}
				break;
			}
		}
	}
}

void recursiveOverrideSprite(AnimationGroup &animGroup, Sprite *sprite)
{
	for (unsigned int i = 0; i < animGroup.anims().size(); i++)
	{
		IAnimation &anim = *animGroup.anims()[i];

		switch (anim.type())
		{
			case IAnimation::Type::PARALLEL_GROUP:
			case IAnimation::Type::SEQUENTIAL_GROUP:
			{
				AnimationGroup &innerGroup = static_cast<AnimationGroup &>(anim);
				recursiveOverrideSprite(innerGroup, sprite);
				break;
			}
			case IAnimation::Type::PROPERTY:
			{
				PropertyAnimation &propertyAnim = static_cast<PropertyAnimation &>(anim);
				if (propertyAnim.sprite() != sprite)
					propertyAnim.setSprite(sprite);
				break;
			}
			case IAnimation::Type::GRID:
			{
				GridAnimation &gridAnim = static_cast<GridAnimation &>(anim);
				if (gridAnim.sprite() != sprite)
					gridAnim.setSprite(sprite);
				break;
			}
			case IAnimation::Type::SCRIPT:
			{
				ScriptAnimation &scriptAnim = static_cast<ScriptAnimation &>(anim);
				if (scriptAnim.sprite() != sprite)
					scriptAnim.setSprite(sprite);
				break;
			}
		}
	}
}

bool recursiveCheckAnimationSprite(AnimationGroup &animGroup, const Sprite *sprite)
{
	for (int i = animGroup.anims().size() - 1; i >= 0; i--)
	{
		IAnimation &anim = *animGroup.anims()[i];

		switch (anim.type())
		{
			case IAnimation::Type::PARALLEL_GROUP:
			case IAnimation::Type::SEQUENTIAL_GROUP:
			{
				AnimationGroup &innerGroup = static_cast<AnimationGroup &>(anim);
				return recursiveCheckAnimationSprite(innerGroup, sprite);
				break;
			}
			case IAnimation::Type::PROPERTY:
			{
				PropertyAnimation &propertyAnim = static_cast<PropertyAnimation &>(anim);
				if (propertyAnim.sprite() != sprite)
					return false;
				break;
			}
			case IAnimation::Type::GRID:
			{
				GridAnimation &gridAnim = static_cast<GridAnimation &>(anim);
				if (gridAnim.sprite() != sprite)
					return false;
				break;
			}
			case IAnimation::Type::SCRIPT:
			{
				ScriptAnimation &scriptAnim = static_cast<ScriptAnimation &>(anim);
				if (scriptAnim.sprite() != sprite)
					return false;
				break;
			}
		}
	}

	return true;
}

void recursiveCloneSpriteAnimations(AnimationGroup &animGroup, const Sprite *fromSprite, Sprite *toSprite)
{
	// Reverse for loop to add cloned animations after the original ones
	for (int i = animGroup.anims().size() - 1; i >= 0; i--)
	{
		IAnimation &anim = *animGroup.anims()[i];

		switch (anim.type())
		{
			case IAnimation::Type::PARALLEL_GROUP:
			case IAnimation::Type::SEQUENTIAL_GROUP:
			{
				AnimationGroup &innerGroup = static_cast<AnimationGroup &>(anim);
				// If the group only contains animations associated to the same sprite then create a new group
				const bool createNewGroup = recursiveCheckAnimationSprite(innerGroup, fromSprite);
				if (createNewGroup)
				{
					animGroup.anims().insertAt(i + 1, innerGroup.clone());
					AnimationGroup &clonedGroup = static_cast<AnimationGroup &>(*animGroup.anims()[i + 1]);
					recursiveOverrideSprite(clonedGroup, toSprite);
				}
				else
					recursiveCloneSpriteAnimations(innerGroup, fromSprite, toSprite);

				break;
			}
			case IAnimation::Type::PROPERTY:
			{
				PropertyAnimation &propertyAnim = static_cast<PropertyAnimation &>(anim);
				if (propertyAnim.sprite() == fromSprite)
				{
					animGroup.anims().insertAt(i + 1, propertyAnim.clone());
					PropertyAnimation &clonedAnim = static_cast<PropertyAnimation &>(*animGroup.anims()[i + 1]);
					clonedAnim.setSprite(toSprite);
					// Retain locked flag as the cloned animation has been assigned to a different sprite
					clonedAnim.setLocked(propertyAnim.isLocked());
				}
				break;
			}
			case IAnimation::Type::GRID:
			{
				GridAnimation &gridAnim = static_cast<GridAnimation &>(anim);
				if (gridAnim.sprite() == fromSprite)
				{
					animGroup.anims().insertAt(i + 1, gridAnim.clone());
					GridAnimation &clonedAnim = static_cast<GridAnimation &>(*animGroup.anims()[i + 1]);
					clonedAnim.setSprite(toSprite);
					// Retain locked flag as the cloned animation has been assigned to a different sprite
					clonedAnim.setLocked(gridAnim.isLocked());
				}
				break;
			}
			case IAnimation::Type::SCRIPT:
			{
				ScriptAnimation &scriptAnim = static_cast<ScriptAnimation &>(anim);
				if (scriptAnim.sprite() == fromSprite)
				{
					animGroup.anims().insertAt(i + 1, scriptAnim.clone());
					ScriptAnimation &clonedAnim = static_cast<ScriptAnimation &>(*animGroup.anims()[i + 1]);
					clonedAnim.setSprite(toSprite);
					// Retain locked flag as the cloned animation has been assigned to a different sprite
					clonedAnim.setLocked(scriptAnim.isLocked());
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
    : speedMultiplier_(1.0f), animGroup_(nctl::makeUnique<ParallelAnimationGroup>())
{
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void AnimationManager::update(float deltaTime)
{
	animGroup_->update(deltaTime * speedMultiplier_);
}

void AnimationManager::clear()
{
	animGroup_->anims().clear();
}

void AnimationManager::removeAnimation(IAnimation *anim)
{
	if (anim != nullptr)
		recursiveRemoveAnimation(*anim);
}

void AnimationManager::removeSprite(Sprite *sprite)
{
	if (sprite != nullptr)
		recursiveRemoveSprite(*animGroup_, sprite);
}

void AnimationManager::assignGridAnchorToParameters(Sprite *sprite)
{
	recursiveAssignGridAnchorToParameters(*animGroup_, sprite);
}

void AnimationManager::removeScript(Script *script)
{
	if (script != nullptr)
		recursiveRemoveScript(*animGroup_, script);
}

void AnimationManager::reloadScript(Script *script)
{
	if (script != nullptr)
		recursiveReloadScript(*animGroup_, script);
}

void AnimationManager::initScriptsForSprite(Sprite *sprite)
{
	if (sprite != nullptr)
		recursiveInitScriptsForSprite(*animGroup_, sprite);
}

void AnimationManager::overrideSprite(AnimationGroup &animGroup, Sprite *sprite)
{
	if (sprite != nullptr)
		recursiveOverrideSprite(animGroup, sprite);
}

void AnimationManager::cloneSpriteAnimations(const Sprite *fromSprite, Sprite *toSprite)
{
	if (fromSprite != nullptr && toSprite != nullptr && fromSprite != toSprite)
		recursiveCloneSpriteAnimations(*animGroup_, fromSprite, toSprite);
}
