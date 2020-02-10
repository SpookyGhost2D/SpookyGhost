#ifndef CLASS_SERIALIZERS
#define CLASS_SERIALIZERS

#include <nctl/UniquePtr.h>

class LuaSerializer;
class Canvas;
class Texture;
class Sprite;

class IAnimation;
class PropertyAnimation;
class GridAnimation;

class SaveAnim;

namespace Serializers {

void serialize(LuaSerializer &ls, const char *name, const Canvas &canvas);
void serialize(LuaSerializer &ls, const Texture &texture);
void serialize(LuaSerializer &ls, const Sprite &sprite);

void serialize(LuaSerializer &ls, const IAnimation *anim);
void serialize(LuaSerializer &ls, const PropertyAnimation &anim);
void serialize(LuaSerializer &ls, const GridAnimation &anim);

void serialize(LuaSerializer &ls, const char *name, const SaveAnim &saveAnim);

}

namespace Deserializers {

void deserialize(LuaSerializer &ls, const char *name, Canvas &canvas);
void deserialize(LuaSerializer &ls, nctl::UniquePtr<Texture> &texture);
void deserialize(LuaSerializer &ls, nctl::UniquePtr<Sprite> &sprite);
void deserialize(LuaSerializer &ls, nctl::UniquePtr<IAnimation> &anim);
void deserialize(LuaSerializer &ls, const char *name, SaveAnim &saveAnim);

}

#endif
