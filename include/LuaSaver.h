#ifndef CLASS_LUASAVER
#define CLASS_LUASAVER

#include <nctl/UniquePtr.h>

class LuaSerializer;
class UserInterface;
class Canvas;
class SpriteManager;
class AnimationManager;
class SaveAnim;

/// The class that helps with Lua serialization
class LuaSaver
{
  public:
	struct Data
	{
		Data(Canvas &ca, SpriteManager &sm, AnimationManager &am, SaveAnim &sa)
		    : canvas(ca), spriteMgr(sm), animMgr(am), saveAnim(sa) {}

		Canvas &canvas;
		SpriteManager &spriteMgr;
		AnimationManager &animMgr;
		SaveAnim &saveAnim;
	};

	LuaSaver();

	void load(const char *filename, Data &data);
	void save(const char *filename, const Data &data);

  private:
	nctl::UniquePtr<LuaSerializer> serializer_;
};

#endif
