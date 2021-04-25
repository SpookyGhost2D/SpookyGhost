#ifndef CLASS_SCRIPT
#define CLASS_SCRIPT

#include <nctl/String.h>
#include <ncine/LuaStateManager.h>

struct lua_State;

class Sprite;

namespace nc = ncine;

/// The class representing a single Lua script
class Script
{
  public:
	Script();
	Script(const char *filename);

	inline bool canRun() const { return canRun_; }

	inline const nctl::String &name() const { return name_; }
	inline void setName(const nctl::String &name) { name_ = name; }

	inline const char *errorMsg() const { return errorMessage_.data(); }

	bool load(const char *filename);
	bool reload();

  private:
	bool canRun_;
	nctl::String name_;
	nctl::String errorMessage_;
	nc::LuaStateManager luaState_;

	bool run(const char *filename, const char *chunkName);

	friend class ScriptAnimation;
};

#endif
