#include <ncine/LuaUtils.h>
#include <ncine/LuaDebug.h>
#include "ScriptAnimation.h"
#include "Script.h"
#include "ScriptManager.h"
#include "Sprite.h"

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

ScriptAnimation::ScriptAnimation()
    : ScriptAnimation(nullptr, nullptr)
{
}

ScriptAnimation::ScriptAnimation(Sprite *sprite, Script *script)
    : CurveAnimation(EasingCurve::Type::LINEAR, EasingCurve::LoopMode::DISABLED),
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

	anim->name.assign(name);
	anim->enabled = enabled;
	// Animation state is not cloned
	anim->setParent(parent_);

	// Always disable locking for cloned animations
	anim->isLocked_ = false;
	anim->curve_ = curve_;
	anim->speed_ = speed_;

	return nctl::move(anim);
}

void ScriptAnimation::play()
{
	CurveAnimation::play();
	if (sprite_)
		runScript("init", curve_.value());
}

void ScriptAnimation::stop()
{
	CurveAnimation::stop();
	if (sprite_)
		runScript("update", curve_.value());
}

void ScriptAnimation::update(float deltaTime)
{
	switch (state_)
	{
		case State::STOPPED:
		case State::PAUSED:
			if (isLocked_ && sprite_ && sprite_->visible)
				runScript("update", curve_.value());
			break;
		case State::PLAYING:
		{
			curve_.next(speed_ * deltaTime);
			if (sprite_ && sprite_->visible)
				runScript("update", curve_.value());
			break;
		}
	}

	CurveAnimation::update(deltaTime);
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
	return true;
}
