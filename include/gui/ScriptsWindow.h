#ifndef CLASS_SCRIPTSWINDOW
#define CLASS_SCRIPTSWINDOW

#ifdef __EMSCRIPTEN__
	#include <ncine/EmscriptenLocalFile.h>
	namespace nc = ncine;
#endif

class UserInterface;

/// The scripts window class
class ScriptsWindow
{
  public:
	explicit ScriptsWindow(UserInterface &ui);

	void create();
	bool loadScript(const char *filename);
	void reloadScript();
#ifdef __EMSCRIPTEN__
	bool loadScript(const char *bufferName, const char *bufferPtr, unsigned long int bufferSize);
	void reloadScript(const char *bufferName, const char *bufferPtr, unsigned long int bufferSize);
#endif

  private:
	UserInterface &ui_;

#ifdef __EMSCRIPTEN__
	nc::EmscriptenLocalFile loadScriptLocalFile_;
	nc::EmscriptenLocalFile reloadScriptLocalFile_;
#endif

	void removeScript();
};

#endif
