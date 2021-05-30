#include <ncine/LuaUtils.h>
#include <ncine/LuaDebug.h>
#include "ScriptAnimation.h"
#include "Script.h"
#include "ScriptManager.h"
#include "Sprite.h"
#include "AnimationGroup.h"

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

ScriptAnimation::ScriptAnimation()
    : ScriptAnimation(nullptr, nullptr)
{
}

ScriptAnimation::ScriptAnimation(Sprite *sprite, Script *script)
    : CurveAnimation(EasingCurve::Type::LINEAR, Loop::Mode::DISABLED),
      sprite_(nullptr), script_(nullptr)
{
	setSprite(sprite);
	setScript(script);

	// Locking is disabled to limit the number of script calls
	isLocked_ = false;
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

nctl::UniquePtr<IAnimation> ScriptAnimation::clone() const
{
	nctl::UniquePtr<ScriptAnimation> anim = nctl::makeUnique<ScriptAnimation>(sprite_, script_);
	CurveAnimation::cloneTo(*anim);

	return nctl::move(anim);
}

void ScriptAnimation::play()
{
	CurveAnimation::play();
	if (sprite_)
		runScript("init", curve_.value());
}

void ScriptAnimation::perform()
{
	if (sprite_ && sprite_->visible)
		runScript("update", curve_.value());
}

void ScriptAnimation::setSprite(Sprite *sprite)
{
	if (sprite_ != sprite)
	{
		if (sprite_)
			sprite_->decrementGridAnimCounter();
		if (sprite)
			sprite->incrementGridAnimCounter();

		sprite_ = sprite;
	}
}

void ScriptAnimation::setScript(Script *script)
{
	script_ = script;
}

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

bool ScriptAnimation::runScript(const char *functionName, float value)
{
	if (sprite_ == nullptr || script_ == nullptr)
		return false;

	lua_State *L = script_->luaState_.state();
	const int type = nc::LuaUtils::getGlobal(L, functionName);

	if (nc::LuaUtils::isFunction(type))
	{
		ScriptManager::pushSprite(L, sprite_);
		nc::LuaUtils::push(L, value);
		const int status = nc::LuaUtils::pcall(L, 1, 0);
		if (nc::LuaUtils::isStatusOk(status) == false)
		{
			LOGE_X("Error running \"%s\" function for script \"%s\" (%s):\n%s", functionName, script_->name().data(),
			       nc::LuaDebug::statusToString(status), nc::LuaUtils::retrieve<const char *>(L, -1));
			nc::LuaUtils::pop(L);
		}
	}
	else
		nc::LuaUtils::pop(L);

	return true;
}
