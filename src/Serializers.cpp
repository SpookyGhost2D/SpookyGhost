#include <ncine/FileSystem.h>
#include "Canvas.h"
#include "Texture.h"
#include "Sprite.h"
#include "SpriteManager.h"
#include "Script.h"
#include "ScriptManager.h"
#include "ParallelAnimationGroup.h"
#include "SequentialAnimationGroup.h"
#include "PropertyAnimation.h"
#include "GridAnimation.h"
#include "GridFunction.h"
#include "ScriptAnimation.h"
#include "AnimationManager.h"
#include "Configuration.h"
#include "singletons.h"

#include "Serializers.h"
#include "LuaSerializer.h"
#include "SerializerContext.h"

namespace Serializers {

void serialize(LuaSerializer &ls, const char *name, const Canvas &canvas)
{
	ls.buffer().formatAppend("%s =\n", name);
	ls.buffer().append("{\n");
	ls.indent();

	serialize(ls, "size", canvas.size());
	serialize(ls, "background_color", canvas.backgroundColor);

	ls.unindent();
	ls.buffer().append("}\n");
}

void serialize(LuaSerializer &ls, const Texture &texture)
{
	ls.buffer().append("{\n");
	ls.indent();

	serialize(ls, "name", texture.name());

	ls.unindent();
	ls.buffer().append("},\n");
}

void serialize(LuaSerializer &ls, const char *name, Sprite::BlendingPreset blendingPreset)
{
	switch (blendingPreset)
	{
		case Sprite::BlendingPreset::DISABLED:
			serialize(ls, name, "disabled");
			break;
		case Sprite::BlendingPreset::ALPHA:
			serialize(ls, name, "alpha");
			break;
		case Sprite::BlendingPreset::PREMULTIPLIED_ALPHA:
			serialize(ls, name, "premultiplied_alpha");
			break;
		case Sprite::BlendingPreset::ADDITIVE:
			serialize(ls, name, "additive");
			break;
		case Sprite::BlendingPreset::MULTIPLY:
			serialize(ls, name, "multiply");
			break;
	}
}

void serialize(LuaSerializer &ls, const SpriteEntry *spriteEntry)
{
	ls.buffer().append("{\n");
	ls.indent();

	switch (spriteEntry->type())
	{
		case SpriteEntry::Type::GROUP:
		{
			const SpriteGroup &spriteGroup = *spriteEntry->toGroup();
			serialize(ls, "type", "sprite_group");
			serialize(ls, spriteGroup);
			break;
		}
		case SpriteEntry::Type::SPRITE:
		{
			const Sprite &sprite = *spriteEntry->toSprite();
			serialize(ls, "type", "sprite");
			serialize(ls, sprite);
			break;
		}
	}

	ls.unindent();
	ls.buffer().append("},\n");
}

void serialize(LuaSerializer &ls, const SpriteGroup &spriteGroup)
{
	SerializerContext *context = static_cast<SerializerContext *>(ls.context());

	serializePtr(ls, "parent_group", static_cast<const SpriteEntry *>(spriteGroup.parentGroup()), *context->spriteEntryHash);
	serialize(ls, "entry_color", spriteGroup.entryColor());
	serialize(ls, "name", spriteGroup.name());
}

void serialize(LuaSerializer &ls, const Sprite &sprite)
{
	SerializerContext *context = static_cast<SerializerContext *>(ls.context());

	serializePtr(ls, "parent_group", static_cast<const SpriteEntry *>(sprite.parentGroup()), *context->spriteEntryHash);
	serialize(ls, "entry_color", sprite.entryColor());
	serialize(ls, "name", sprite.name);

	serializePtr(ls, "texture", &sprite.texture(), *context->textureHash);
	serializePtr(ls, "parent", static_cast<const SpriteEntry *>(sprite.parent()), *context->spriteEntryHash);
	serialize(ls, "visible", sprite.visible);
	nc::Vector2f position(sprite.x, sprite.y);
	serialize(ls, "position", position);
	serialize(ls, "rotation", sprite.rotation);
	serialize(ls, "scale_factor", sprite.scaleFactor);
	serialize(ls, "anchor_point", sprite.anchorPoint);
	serialize(ls, "color", sprite.color);
	// Grid anchor point values are serialized by grid animations
	serialize(ls, "texrect", sprite.texRect());
	serialize(ls, "flip_x", sprite.isFlippedX());
	serialize(ls, "flip_y", sprite.isFlippedY());
	serialize(ls, "rgb_blending", sprite.rgbBlendingPreset());
	serialize(ls, "alpha_blending", sprite.alphaBlendingPreset());
}

void serialize(LuaSerializer &ls, const Script &script)
{
	ls.buffer().append("{\n");
	ls.indent();

	serialize(ls, "name", script.name());

	ls.unindent();
	ls.buffer().append("},\n");
}

void serialize(LuaSerializer &ls, const IAnimation *anim)
{
	SerializerContext *context = static_cast<SerializerContext *>(ls.context());

	ls.buffer().append("{\n");
	ls.indent();

	serialize(ls, "name", anim->name.data());
	serialize(ls, "enabled", anim->enabled);
	serializePtr(ls, "parent", static_cast<const IAnimation *>(anim->parent()), *context->animationHash);
	serialize(ls, "delay", anim->delay());

	switch (anim->type())
	{
		case IAnimation::Type::PARALLEL_GROUP:
		{
			const ParallelAnimationGroup &parallelGroup = static_cast<const ParallelAnimationGroup &>(*anim);
			serialize(ls, "type", "parallel_group");
			serialize(ls, parallelGroup);
			break;
		}
		case IAnimation::Type::SEQUENTIAL_GROUP:
		{
			const SequentialAnimationGroup &sequentialGroup = static_cast<const SequentialAnimationGroup &>(*anim);
			serialize(ls, "type", "sequential_group");
			serialize(ls, sequentialGroup);
			break;
		}
		case IAnimation::Type::PROPERTY:
		{
			const PropertyAnimation &propertyAnim = static_cast<const PropertyAnimation &>(*anim);
			serialize(ls, "type", "property_animation");
			serialize(ls, propertyAnim);
			break;
		}
		case IAnimation::Type::GRID:
		{
			const GridAnimation &gridAnimation = static_cast<const GridAnimation &>(*anim);
			serialize(ls, "type", "grid_animation");
			serialize(ls, gridAnimation);
			break;
		}
		case IAnimation::Type::SCRIPT:
		{
			const ScriptAnimation &scriptAnimation = static_cast<const ScriptAnimation &>(*anim);
			serialize(ls, "type", "script_animation");
			serialize(ls, scriptAnimation);
			break;
		}
	}

	ls.unindent();
	ls.buffer().append("},\n");
}

void serialize(LuaSerializer &ls, const LoopComponent &loop)
{
	switch (loop.direction())
	{
		case Loop::Direction::FORWARD:
			serialize(ls, "direction", "forward");
			break;
		case Loop::Direction::BACKWARD:
			serialize(ls, "direction", "backward");
			break;
	}

	switch (loop.mode())
	{
		case Loop::Mode::DISABLED:
			serialize(ls, "loop_mode", "disabled");
			break;
		case Loop::Mode::REWIND:
			serialize(ls, "loop_mode", "rewind");
			break;
		case Loop::Mode::PING_PONG:
			serialize(ls, "loop_mode", "ping_pong");
			break;
	}

	serialize(ls, "loop_delay", loop.delay());
}

void serialize(LuaSerializer &ls, const AnimationGroup &anim)
{
	serialize(ls, anim.loop());
}

void serialize(LuaSerializer &ls, const char *name, EasingCurve::Type type)
{
	switch (type)
	{
		case EasingCurve::Type::LINEAR:
			serialize(ls, name, "linear");
			break;
		case EasingCurve::Type::QUAD:
			serialize(ls, name, "quad");
			break;
		case EasingCurve::Type::CUBIC:
			serialize(ls, name, "cubic");
			break;
		case EasingCurve::Type::QUART:
			serialize(ls, name, "quart");
			break;
		case EasingCurve::Type::QUINT:
			serialize(ls, name, "quint");
			break;
		case EasingCurve::Type::SINE:
			serialize(ls, name, "sine");
			break;
		case EasingCurve::Type::EXPO:
			serialize(ls, name, "expo");
			break;
		case EasingCurve::Type::CIRC:
			serialize(ls, name, "circ");
			break;
	}
}

void serialize(LuaSerializer &ls, const char *name, const EasingCurve &curve)
{
	ls.buffer().formatAppend("%s =\n", name);
	ls.buffer().append("{\n");
	ls.indent();

	serialize(ls, "type", curve.type());
	serialize(ls, curve.loop());
	serialize(ls, "initial_value", curve.initialValue());
	serialize(ls, "initial_value_enabled", curve.hasInitialValue());
	serialize(ls, "start_time", curve.start());
	serialize(ls, "end_time", curve.end());
	serialize(ls, "scale", curve.scale());
	serialize(ls, "shift", curve.shift());

	ls.unindent();
	ls.buffer().append("},\n");
}

void serialize(LuaSerializer &ls, const PropertyAnimation &anim)
{
	SerializerContext *context = static_cast<SerializerContext *>(ls.context());

	serializePtr(ls, "sprite", static_cast<const SpriteEntry *>(anim.sprite()), *context->spriteEntryHash);
	serialize(ls, "speed", anim.speed());
	serialize(ls, "curve", anim.curve());
	serialize(ls, "property_name", anim.propertyName());
}

void serialize(LuaSerializer &ls, const GridAnimation &anim)
{
	SerializerContext *context = static_cast<SerializerContext *>(ls.context());

	serializePtr(ls, "sprite", static_cast<const SpriteEntry *>(anim.sprite()), *context->spriteEntryHash);
	serialize(ls, "speed", anim.speed());
	serialize(ls, "curve", anim.curve());

	if (anim.function() != nullptr)
	{
		serialize(ls, "function_name", anim.function()->name().data());

		ls.buffer().append("parameters =\n");
		ls.buffer().append("{\n");
		ls.indent();

		for (unsigned int i = 0; i < anim.function()->numParameters(); i++)
		{
			ls.buffer().append("{\n");
			ls.indent();

			const char *paramName = anim.function()->parameterName(i);
			serialize(ls, "name", paramName);
			if (anim.function()->parameterType(i) == GridFunction::ParameterType::FLOAT)
				serialize(ls, "value", anim.parameters()[i].value0);
			else if (anim.function()->parameterType(i) == GridFunction::ParameterType::VECTOR2F)
			{
				nc::Vector2f vecParam(anim.parameters()[i].value0, anim.parameters()[i].value1);
				serialize(ls, "value", vecParam);
			}

			ls.unindent();
			ls.buffer().append("},\n");
		}

		ls.unindent();
		ls.buffer().append("},\n");
	}
}

void serialize(LuaSerializer &ls, const ScriptAnimation &anim)
{
	SerializerContext *context = static_cast<SerializerContext *>(ls.context());

	serializePtr(ls, "sprite", static_cast<const SpriteEntry *>(anim.sprite()), *context->spriteEntryHash);
	serialize(ls, "speed", anim.speed());
	serialize(ls, "curve", anim.curve());
	serializePtr(ls, "script", anim.script(), *context->scriptHash);
}

void serialize(LuaSerializer &ls, const Configuration &cfg)
{
	serializeGlobal(ls, "version", cfg.version);
	serializeGlobal(ls, "width", cfg.width);
	serializeGlobal(ls, "height", cfg.height);
	serializeGlobal(ls, "fullscreen", cfg.fullscreen);
	serializeGlobal(ls, "resizable", cfg.resizable);
	serializeGlobal(ls, "vsync", cfg.withVSync);
	serializeGlobal(ls, "frame_limit", cfg.frameLimit);
	serializeGlobal(ls, "canvas_width", cfg.canvasWidth);
	serializeGlobal(ls, "canvas_height", cfg.canvasHeight);
	serializeGlobal(ls, "gui_scaling", cfg.guiScaling);
	serializeGlobal(ls, "startup_project_name", cfg.startupProjectName);
	serializeGlobal(ls, "auto_play_on_start", cfg.autoPlayOnStart);
	serializeGlobal(ls, "projects_path", cfg.projectsPath);
	serializeGlobal(ls, "textures_path", cfg.texturesPath);
	serializeGlobal(ls, "scripts_path", cfg.scriptsPath);
	serializeGlobal(ls, "show_tips_on_start", cfg.showTipsOnStart);

	const unsigned int numPinnedDirectories = cfg.pinnedDirectories.size();
	if (numPinnedDirectories > 0)
	{
		ls.buffer().append("\n");
		Serializers::serialize(ls, "pinned_directories", cfg.pinnedDirectories);
	}
}

}

namespace Deserializers {

template <>
Sprite::BlendingPreset deserialize(LuaSerializer &ls, const char *name)
{
	lua_State *L = ls.luaState();
	nctl::String blendingString = nc::LuaUtils::retrieveField<const char *>(L, -1, name);

	if (blendingString == "disabled")
		return Sprite::BlendingPreset::DISABLED;
	else if (blendingString == "alpha")
		return Sprite::BlendingPreset::ALPHA;
	else if (blendingString == "premultiplied_alpha")
		return Sprite::BlendingPreset::PREMULTIPLIED_ALPHA;
	else if (blendingString == "additive")
		return Sprite::BlendingPreset::ADDITIVE;
	else if (blendingString == "multiply")
		return Sprite::BlendingPreset::MULTIPLY;
	else
		return Sprite::BlendingPreset::DISABLED;
}

bool deserialize(LuaSerializer &ls, const char *name, Canvas &canvas)
{
	lua_State *L = ls.luaState();
	if (nc::LuaUtils::tryRetrieveGlobalTable(L, name) == false)
		return false;

	canvas.backgroundColor = deserialize<nc::Colorf>(ls, "background_color");
	canvas.resizeTexture(deserialize<nc::Vector2i>(ls, "size"));

	nc::LuaUtils::pop(L);
	return true;
}

void deserialize(LuaSerializer &ls, nctl::UniquePtr<Texture> &texture)
{
	const char *textureName = deserialize<const char *>(ls, "name");
	static nctl::String texturePath(256);

	// Check first if the filename is relative to the configuration textures directory
	texturePath = nc::fs::joinPath(theCfg.texturesPath, textureName);
	if (nc::fs::isReadableFile(texturePath.data()) == false)
	{
		// Then check if the filename is relative to the data textures directory
		texturePath = nc::fs::joinPath(ui::texturesDataDir, textureName);
		// If not then use the full path
		if (nc::fs::isReadableFile(texturePath.data()) == false)
			texturePath = textureName;
	}

	texture = nctl::makeUnique<Texture>(texturePath.data());
	// Set the texture name to its basename to allow for relocatable project files
	texture->setName(textureName);
}

template <>
SpriteEntry::Type deserialize(LuaSerializer &ls, const char *name)
{
	lua_State *L = ls.luaState();
	nctl::String spriteEntryTypeString = nc::LuaUtils::retrieveField<const char *>(L, -1, name);

	if (spriteEntryTypeString == "sprite_group")
		return SpriteEntry::Type::GROUP;
	else if (spriteEntryTypeString == "sprite")
		return SpriteEntry::Type::SPRITE;
	else
		return SpriteEntry::Type::SPRITE;
}

void deserialize(LuaSerializer &ls, nctl::UniquePtr<SpriteEntry> &spriteEntry)
{
	DeserializerContext *context = static_cast<DeserializerContext *>(ls.context());

	SpriteEntry::Type type = SpriteEntry::Type::SPRITE;
	if (context->version >= 7)
		type = deserialize<SpriteEntry::Type>(ls, "type");

	switch (type)
	{
		case SpriteEntry::Type::GROUP:
		{
			nctl::UniquePtr<SpriteGroup> spriteGroup = nctl::makeUnique<SpriteGroup>();
			deserialize(ls, spriteGroup);
			spriteEntry = nctl::move(spriteGroup);
			break;
		}
		case SpriteEntry::Type::SPRITE:
		{
			Texture *texture = deserializePtr(ls, "texture", *context->textures);
			FATAL_ASSERT(texture);
			nctl::UniquePtr<Sprite> sprite = nctl::makeUnique<Sprite>(texture);

			deserialize(ls, sprite);
			spriteEntry = nctl::move(sprite);
			break;
		}
	}

	SpriteGroup *parentGroup = nullptr;
	if (context->version >= 7)
	{
		parentGroup = static_cast<SpriteGroup *>(deserializePtr(ls, "parent_group", *context->spriteEntries));
		spriteEntry->entryColor() = deserialize<nc::Colorf>(ls, "entry_color");
	}
	parentGroup = (parentGroup == nullptr) ? &theSpriteMgr->root() : parentGroup;
	spriteEntry->setParentGroup(parentGroup);
}

void deserialize(LuaSerializer &ls, nctl::UniquePtr<SpriteGroup> &spriteGroup)
{
	deserialize(ls, "name", spriteGroup->name());
}

void deserialize(LuaSerializer &ls, nctl::UniquePtr<Sprite> &sprite)
{
	DeserializerContext *context = static_cast<DeserializerContext *>(ls.context());

	deserialize(ls, "name", sprite->name);
	Sprite *parent = static_cast<Sprite *>(deserializePtr(ls, "parent", *context->spriteEntries));
	sprite->setParent(parent);
	deserialize(ls, "visible", sprite->visible);
	nc::Vector2f position = deserialize<nc::Vector2f>(ls, "position");
	sprite->x = position.x;
	sprite->y = position.y;
	deserialize(ls, "rotation", sprite->rotation);
	deserialize(ls, "scale_factor", sprite->scaleFactor);
	deserialize(ls, "anchor_point", sprite->anchorPoint);
	deserialize(ls, "color", sprite->color);
	// Grid anchor point values are deserialized by grid animations
	sprite->setTexRect(deserialize<nc::Recti>(ls, "texrect"));
	sprite->setFlippedX(deserialize<bool>(ls, "flip_x"));
	sprite->setFlippedY(deserialize<bool>(ls, "flip_y"));

	if (context->version >= 5)
	{
		sprite->setRgbBlendingPreset(deserialize<Sprite::BlendingPreset>(ls, "rgb_blending"));
		sprite->setAlphaBlendingPreset(deserialize<Sprite::BlendingPreset>(ls, "alpha_blending"));
	}
	else
	{
		sprite->setRgbBlendingPreset(deserialize<Sprite::BlendingPreset>(ls, "blending"));
		sprite->setAlphaBlendingPreset(deserialize<Sprite::BlendingPreset>(ls, "blending"));
	}
}

void deserialize(LuaSerializer &ls, nctl::UniquePtr<Script> &script)
{
	const char *scriptName = deserialize<const char *>(ls, "name");
	static nctl::String scriptPath(256);

	// Check first if the filename is relative to the configuration scripts directory
	scriptPath = nc::fs::joinPath(theCfg.scriptsPath, scriptName);
	if (nc::fs::isReadableFile(scriptPath.data()) == false)
	{
		// Then check if the filename is relative to the data scripts directory
		scriptPath = nc::fs::joinPath(ui::scriptsDataDir, scriptName);
		// If not then use the full path
		if (nc::fs::isReadableFile(scriptPath.data()) == false)
			scriptPath = scriptName;
	}

	script = nctl::makeUnique<Script>(scriptPath.data());
	// Set the script name to its basename to allow for relocatable project files
	script->setName(scriptName);
}

template <>
IAnimation::Type deserialize(LuaSerializer &ls, const char *name)
{
	lua_State *L = ls.luaState();
	nctl::String animTypeString = nc::LuaUtils::retrieveField<const char *>(L, -1, name);

	if (animTypeString == "parallel_group")
		return IAnimation::Type::PARALLEL_GROUP;
	else if (animTypeString == "sequential_group")
		return IAnimation::Type::SEQUENTIAL_GROUP;
	else if (animTypeString == "property_animation")
		return IAnimation::Type::PROPERTY;
	else if (animTypeString == "grid_animation")
		return IAnimation::Type::GRID;
	else if (animTypeString == "script_animation")
		return IAnimation::Type::SCRIPT;
	else
		return IAnimation::Type::PARALLEL_GROUP;
}

void deserialize(LuaSerializer &ls, LoopComponent &loop)
{
	DeserializerContext *context = static_cast<DeserializerContext *>(ls.context());
	lua_State *L = ls.luaState();

	nctl::String directionString = nc::LuaUtils::retrieveField<const char *>(L, -1, "direction");
	if (directionString == "forward")
		loop.setDirection(Loop::Direction::FORWARD);
	else if (directionString == "backward")
		loop.setDirection(Loop::Direction::BACKWARD);
	else
		loop.setDirection(Loop::Direction::FORWARD);

	nctl::String loopModeString = nc::LuaUtils::retrieveField<const char *>(L, -1, "loop_mode");
	if (loopModeString == "disabled")
		loop.setMode(Loop::Mode::DISABLED);
	else if (loopModeString == "rewind")
		loop.setMode(Loop::Mode::REWIND);
	else if (loopModeString == "ping_pong")
		loop.setMode(Loop::Mode::PING_PONG);
	else
		loop.setMode(Loop::Mode::DISABLED);

	if (context->version >= 4)
	{
		const float loopDelay = nc::LuaUtils::retrieveField<float>(L, -1, "loop_delay");
		loop.setDelay(loopDelay);
	}
}

void deserialize(LuaSerializer &ls, AnimationGroup *anim)
{
	deserialize(ls, anim->loop());
}

template <>
EasingCurve::Type deserialize(LuaSerializer &ls, const char *name)
{
	lua_State *L = ls.luaState();
	nctl::String easingTypeString = nc::LuaUtils::retrieveField<const char *>(L, -1, name);

	if (easingTypeString == "linear")
		return EasingCurve::Type::LINEAR;
	else if (easingTypeString == "quad")
		return EasingCurve::Type::QUAD;
	else if (easingTypeString == "cubic")
		return EasingCurve::Type::CUBIC;
	else if (easingTypeString == "quart")
		return EasingCurve::Type::QUART;
	else if (easingTypeString == "quint")
		return EasingCurve::Type::QUINT;
	else if (easingTypeString == "sine")
		return EasingCurve::Type::SINE;
	else if (easingTypeString == "expo")
		return EasingCurve::Type::EXPO;
	else if (easingTypeString == "circ")
		return EasingCurve::Type::CIRC;
	else
		return EasingCurve::Type::LINEAR;
}

void deserialize(LuaSerializer &ls, const char *name, EasingCurve &curve)
{
	ASSERT(name);
	DeserializerContext *context = static_cast<DeserializerContext *>(ls.context());
	lua_State *L = ls.luaState();

	nc::LuaUtils::retrieveFieldTable(L, -1, name);

	curve.setType(deserialize<EasingCurve::Type>(ls, "type"));
	deserialize(ls, curve.loop());
	if (context->version >= 4)
	{
		curve.setInitialValue(deserialize<float>(ls, "initial_value"));
		curve.enableInitialValue(deserialize<bool>(ls, "initial_value_enabled"));
	}
	curve.setStart(deserialize<float>(ls, "start_time"));
	curve.setEnd(deserialize<float>(ls, "end_time"));
	curve.setScale(deserialize<float>(ls, "scale"));
	curve.setShift(deserialize<float>(ls, "shift"));

	nc::LuaUtils::pop(L);
}

void deserialize(LuaSerializer &ls, nctl::UniquePtr<PropertyAnimation> &anim)
{
	DeserializerContext *context = static_cast<DeserializerContext *>(ls.context());

	Sprite *sprite = static_cast<Sprite *>(deserializePtr(ls, "sprite", *context->spriteEntries));

	anim->setSprite(sprite);
	anim->setSpeed(deserialize<float>(ls, "speed"));
	deserialize(ls, "curve", anim->curve());
	const char *propertyName = deserialize<const char *>(ls, "property_name");
	anim->setProperty(propertyName);
}

void deserialize(LuaSerializer &ls, nctl::UniquePtr<GridAnimation> &anim)
{
	DeserializerContext *context = static_cast<DeserializerContext *>(ls.context());

	Sprite *sprite = static_cast<Sprite *>(deserializePtr(ls, "sprite", *context->spriteEntries));

	anim->setSprite(sprite);
	anim->setSpeed(deserialize<float>(ls, "speed"));
	deserialize(ls, "curve", anim->curve());

	const char *functionName = deserialize<const char *>(ls, "function_name");
	if (functionName == nullptr)
		return;

	const GridFunction **functionPtr = context->functionHash->find(functionName);
	if (functionPtr)
	{
		const GridFunction *function = *functionPtr;
		anim->setFunction(function);
		for (Array ar(ls, "parameters"); ar.hasNext(); ar.next())
		{
			const unsigned int i = ar.index();
			const char *paramName = deserialize<const char *>(ls, "name");
			if (function->parameterInfo(i).name == paramName)
			{
				if (function->parameterType(i) == GridFunction::ParameterType::FLOAT)
					anim->parameters()[i].value0 = deserialize<float>(ls, "value");
				else if (function->parameterType(i) == GridFunction::ParameterType::VECTOR2F)
				{
					const nc::Vector2f valueVec = deserialize<nc::Vector2f>(ls, "value");
					anim->parameters()[i].value0 = valueVec.x;
					anim->parameters()[i].value1 = valueVec.y;
				}
			}
		}
	}
}

void deserialize(LuaSerializer &ls, nctl::UniquePtr<ScriptAnimation> &anim)
{
	DeserializerContext *context = static_cast<DeserializerContext *>(ls.context());

	Sprite *sprite = static_cast<Sprite *>(deserializePtr(ls, "sprite", *context->spriteEntries));

	anim->setSprite(sprite);
	anim->setSpeed(deserialize<float>(ls, "speed"));
	deserialize(ls, "curve", anim->curve());
	Script *script = deserializePtr(ls, "script", *context->scripts);
	anim->setScript(script);
}

void deserialize(LuaSerializer &ls, nctl::UniquePtr<IAnimation> &anim)
{
	DeserializerContext *context = static_cast<DeserializerContext *>(ls.context());

	IAnimation::Type type = deserialize<IAnimation::Type>(ls, "type");

	switch (type)
	{
		case IAnimation::Type::PARALLEL_GROUP:
		{
			nctl::UniquePtr<ParallelAnimationGroup> parallelGroup = nctl::makeUnique<ParallelAnimationGroup>();
			if (context->version >= 6)
				deserialize(ls, parallelGroup.get());
			anim = nctl::move(parallelGroup);
			break;
		}
		case IAnimation::Type::SEQUENTIAL_GROUP:
		{
			nctl::UniquePtr<SequentialAnimationGroup> sequentialGroup = nctl::makeUnique<SequentialAnimationGroup>();
			deserialize(ls, sequentialGroup.get());
			anim = nctl::move(sequentialGroup);
			break;
		}
		case IAnimation::Type::PROPERTY:
		{
			nctl::UniquePtr<PropertyAnimation> propertyAnim = nctl::makeUnique<PropertyAnimation>();
			deserialize(ls, propertyAnim);
			anim = nctl::move(propertyAnim);
			break;
		}
		case IAnimation::Type::GRID:
		{
			nctl::UniquePtr<GridAnimation> gridAnim = nctl::makeUnique<GridAnimation>();
			deserialize(ls, gridAnim);
			anim = nctl::move(gridAnim);
			break;
		}
		case IAnimation::Type::SCRIPT:
		{
			nctl::UniquePtr<ScriptAnimation> scriptAnim = nctl::makeUnique<ScriptAnimation>();
			deserialize(ls, scriptAnim);
			anim = nctl::move(scriptAnim);
			break;
		}
	}

	deserialize(ls, "name", anim->name);
	if (context->version >= 2)
		deserialize(ls, "enabled", anim->enabled);
	AnimationGroup *parent = static_cast<AnimationGroup *>(deserializePtr(ls, "parent", *context->animations));
	if (parent == nullptr)
		parent = &theAnimMgr->animGroup();
	anim->setParent(parent);
	if (context->version >= 4)
		anim->setDelay(deserialize<float>(ls, "delay"));
}

void deserialize(LuaSerializer &ls, Configuration &cfg)
{
	const int version = deserializeGlobal<int>(ls, "version");
	ASSERT(version >= 1);

	cfg.width = deserializeGlobal<int>(ls, "width");
	cfg.height = deserializeGlobal<int>(ls, "height");
	cfg.fullscreen = deserializeGlobal<bool>(ls, "fullscreen");
	cfg.resizable = deserializeGlobal<bool>(ls, "resizable");
	cfg.withVSync = deserializeGlobal<bool>(ls, "vsync");
	cfg.frameLimit = deserializeGlobal<int>(ls, "frame_limit");
	cfg.canvasWidth = deserializeGlobal<int>(ls, "canvas_width");
	cfg.canvasHeight = deserializeGlobal<int>(ls, "canvas_height");

	if (version >= 2)
		cfg.guiScaling = deserializeGlobal<float>(ls, "gui_scaling");

	if (version >= 3)
	{
		deserializeGlobal(ls, "startup_project_name", cfg.startupProjectName);
		deserializeGlobal(ls, "projects_path", cfg.projectsPath);
		deserializeGlobal(ls, "scripts_path", cfg.scriptsPath);
	}
	else
	{
		deserializeGlobal(ls, "startup_script_name", cfg.startupProjectName);
		deserializeGlobal(ls, "scripts_path", cfg.projectsPath);
	}

	cfg.autoPlayOnStart = deserializeGlobal<bool>(ls, "auto_play_on_start");
	deserializeGlobal(ls, "textures_path", cfg.texturesPath);

	if (version >= 4)
		cfg.showTipsOnStart = deserializeGlobal<bool>(ls, "show_tips_on_start");

	if (version >= 5)
		deserialize(ls, "pinned_directories", cfg.pinnedDirectories);
}

}
