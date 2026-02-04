#ifndef CLASS_SCRIPTSWINDOW
#define CLASS_SCRIPTSWINDOW

class UserInterface;

/// The scripts window class
class ScriptsWindow
{
  public:
	explicit ScriptsWindow(UserInterface &ui);

	void create();
	bool loadScript(const char *filename);
	void reloadScript();

  private:
	UserInterface &ui_;

	void removeScript();
};

#endif
