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
	explicit Script(const char *filename);

	inline bool canRun() const { return canRun_; }

	inline const nctl::String &name() const { return name_; }
	inline void setName(const nctl::String &name) { name_ = name; }

	inline const char *errorMsg() const { return errorMessage_.data(); }

	bool load(const char *filename);
	bool reload();
#ifdef __EMSCRIPTEN__
	bool load(const char *bufferName, const char *bufferPtr, unsigned long int bufferSize);
	bool reload(const char *bufferName, const char *bufferPtr, unsigned long int bufferSize);
#endif

  private:
	bool canRun_;
	nctl::String name_;
	nctl::String errorMessage_;
	nc::LuaStateManager luaState_;

	bool run(const char *filename, const char *chunkName);
#ifdef __EMSCRIPTEN__
	bool run(const char *bufferName, const char *bufferPtr, unsigned long int bufferSize, const char *chunkName);
#endif

	friend class ScriptAnimation;
};

#endif
