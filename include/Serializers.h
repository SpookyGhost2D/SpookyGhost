#ifndef CLASS_SERIALIZERS
#define CLASS_SERIALIZERS

#include <nctl/UniquePtr.h>

class LuaSerializer;
class Canvas;
class Texture;
class SpriteEntry;
class SpriteGroup;
class Sprite;
class Script;

class LoopComponent;
class IAnimation;
class AnimationGroup;
class PropertyAnimation;
class GridAnimation;
class ScriptAnimation;

class SaveAnim;

class Configuration;

namespace Serializers {

void serialize(LuaSerializer &ls, const char *name, const Canvas &canvas);
void serialize(LuaSerializer &ls, const Texture &texture);
void serialize(LuaSerializer &ls, const SpriteEntry *spriteEntry);
void serialize(LuaSerializer &ls, const SpriteGroup &spriteGroup);
void serialize(LuaSerializer &ls, const Sprite &sprite);
void serialize(LuaSerializer &ls, const Script &script);

void serialize(LuaSerializer &ls, const IAnimation *anim);
void serialize(LuaSerializer &ls, const AnimationGroup &anim);
void serialize(LuaSerializer &ls, const PropertyAnimation &anim);
void serialize(LuaSerializer &ls, const GridAnimation &anim);
void serialize(LuaSerializer &ls, const ScriptAnimation &anim);

void serialize(LuaSerializer &ls, const char *name, const SaveAnim &saveAnim);

void serialize(LuaSerializer &ls, const Configuration &cf);

}

namespace Deserializers {

bool deserialize(LuaSerializer &ls, const char *name, Canvas &canvas);
void deserialize(LuaSerializer &ls, nctl::UniquePtr<Texture> &texture);
void deserialize(LuaSerializer &ls, nctl::UniquePtr<SpriteEntry> &spriteEntry);
void deserialize(LuaSerializer &ls, nctl::UniquePtr<SpriteGroup> &spriteGroup);
void deserialize(LuaSerializer &ls, nctl::UniquePtr<Sprite> &sprite);
void deserialize(LuaSerializer &ls, nctl::UniquePtr<Script> &script);
void deserialize(LuaSerializer &ls, nctl::UniquePtr<IAnimation> &anim);
void deserialize(LuaSerializer &ls, LoopComponent &loop);
bool deserialize(LuaSerializer &ls, const char *name, SaveAnim &saveAnim);
void deserialize(LuaSerializer &ls, Configuration &cfg);

}

#endif
