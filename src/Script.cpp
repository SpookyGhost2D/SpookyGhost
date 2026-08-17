#include <ncine/LuaUtils.h>
#include <ncine/FileSystem.h>
#include "singletons.h"
#include "Script.h"
#include "ScriptManager.h"

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

Script::Script()
    : canRun_(false), name_(256), errorMessage_(256),
      luaState_(nc::LuaStateManager::ApiType::NONE,
                nc::LuaStateManager::StatisticsTracking::DISABLED,
                nc::LuaStateManager::StandardLibraries::LOADED)
{
}

Script::Script(const char *filename)
    : Script()
{
	load(filename);
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

bool Script::load(const char *filename)
{
	const bool hasLoaded = nc::fs::isReadableFile(filename);
	if (hasLoaded)
	{
		name_ = filename;
		run(filename, nc::fs::baseName(filename).data());
	}

	return hasLoaded;
}

bool Script::reload()
{
	nctl::String filename = name_;
	// Resolve relative path made to allow for relocatable project files
	if (nc::fs::isReadableFile(nc::fs::joinPath(theCfg.scriptsPath, name_.data()).data()))
		filename = nc::fs::joinPath(theCfg.scriptsPath, name_.data());

	const bool hasLoaded = nc::fs::isReadableFile(filename.data());
	if (hasLoaded)
	{
		luaState_.reopen();
		run(filename.data(), name_.data());
	}

	return hasLoaded;
}

#ifdef __EMSCRIPTEN__
bool Script::load(const char *bufferName, const char *bufferPtr, unsigned long int bufferSize)
{
	name_ = bufferName;
	return run(bufferName, bufferPtr, bufferSize, nc::fs::baseName(bufferName).data());
}

bool Script::reload(const char *bufferName, const char *bufferPtr, unsigned long int bufferSize)
{
	luaState_.reopen();
	// The name is intentionally left unchanged, so it keeps identifying the script for relocatable project files
	return run(bufferName, bufferPtr, bufferSize, name_.data());
}
#endif

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

bool Script::run(const char *filename, const char *chunkName)
{
	ScriptManager::exposeConstants(luaState_.state());
	ScriptManager::exposeFunctions(luaState_.state());
	canRun_ = luaState_.runFromFile(filename, chunkName, &errorMessage_);

	return canRun_;
}

#ifdef __EMSCRIPTEN__
bool Script::run(const char *bufferName, const char *bufferPtr, unsigned long int bufferSize, const char *chunkName)
{
	ScriptManager::exposeConstants(luaState_.state());
	ScriptManager::exposeFunctions(luaState_.state());
	canRun_ = luaState_.runFromMemory(bufferName, bufferPtr, bufferSize, &errorMessage_);

	return canRun_;
}
#endif
