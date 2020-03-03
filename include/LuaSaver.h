#ifndef CLASS_LUASAVER
#define CLASS_LUASAVER

#include <nctl/UniquePtr.h>

class LuaSerializer;
class UserInterface;
class Canvas;
class SpriteManager;
class AnimationManager;
struct Configuration;

/// The class that helps with Lua serialization
class LuaSaver
{
  public:
	struct Data
	{
		Data(Canvas &ca, SpriteManager &sm, AnimationManager &am)
		    : canvas(ca), spriteMgr(sm), animMgr(am) {}

		Canvas &canvas;
		SpriteManager &spriteMgr;
		AnimationManager &animMgr;
	};

	LuaSaver(unsigned int bufferSize);

	bool load(const char *filename, Data &data);
	void save(const char *filename, const Data &data);

	bool loadCfg(const char *filename, Configuration &cfg);
	void saveCfg(const char *filename, const Configuration &cfg);

  private:
	nctl::UniquePtr<LuaSerializer> serializer_;
};

#endif
