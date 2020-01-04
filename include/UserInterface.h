#ifndef CLASS_USERINTERFACE
#define CLASS_USERINTERFACE

#include <nctl/String.h>

class Canvas;
class AnimationManager;


/// The ImGui user interface class
class UserInterface
{
  public:
	UserInterface(Canvas &canvas, AnimationManager &animMgr);

	void createGuiMainWindow();

  private:
	static const unsigned int MaxStringLength = 256;

	nctl::String auxString_;
	nctl::String filename_;
	Canvas &canvas_;
	AnimationManager &animMgr_;
};

#endif
