#ifndef CLASS_LUASAVER
#define CLASS_LUASAVER

#include <nctl/UniquePtr.h>

class LuaSerializer;
class UserInterface;
class Canvas;
class SpriteManager;
class AnimationManager;
class SaveAnim;
struct Configuration;

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

	LuaSaver(unsigned int bufferSize);

	bool load(const char *filename, Data &data);
	void save(const char *filename, const Data &data);

	bool loadCfg(const char *filename, Configuration &cfg);
	void saveCfg(const char *filename, const Configuration &cfg);

  private:
	nctl::UniquePtr<LuaSerializer> serializer_;
};

#endif