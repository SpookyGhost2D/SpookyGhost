#ifndef CLASS_LUASAVER
#define CLASS_LUASAVER

#include <nctl/UniquePtr.h>
#include <nctl/String.h>

class LuaSerializer;
class UserInterface;
class Canvas;
class SpriteManager;
class ScriptManager;
class AnimationManager;
struct Configuration;

/// The class that helps with Lua serialization
class LuaSaver
{
  public:
	struct Data
	{
		Data(Canvas &ca, SpriteManager &spm, ScriptManager &scm, AnimationManager &am)
		    : canvas(ca), spriteMgr(spm), scriptMgr(scm), animMgr(am) {}

		Canvas &canvas;
		SpriteManager &spriteMgr;
		ScriptManager &scriptMgr;
		AnimationManager &animMgr;
	};

	LuaSaver(unsigned int bufferSize);

	bool load(const char *filename, Data &data);
	void save(const char *filename, const Data &data);

	bool loadCfg(const char *filename, Configuration &cfg);
	void saveCfg(const char *filename, const Configuration &cfg);

	/// Returns the default configuration file
	static const nctl::String &defaultCfgFile() { return defaultCfgFile_; }
	/// Sets the default configuration file
	static void setDefaultCfgFile(const char *filename) { defaultCfgFile_ = filename; }

	/// Loads the configuration using the default file
	inline bool loadCfg(Configuration &cfg) { return loadCfg(defaultCfgFile_.data(), cfg); }
	/// Saves the configuration using the default file
	inline void saveCfg(const Configuration &cfg) { saveCfg(defaultCfgFile_.data(), cfg); }

  private:
	nctl::UniquePtr<LuaSerializer> serializer_;
	static nctl::String defaultCfgFile_;
};

#endif
