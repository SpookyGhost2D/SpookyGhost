#ifndef CLASS_MYEVENTHANDLER
#define CLASS_MYEVENTHANDLER

#include <ncine/IAppEventHandler.h>
#include <ncine/IInputEventHandler.h>
#include <nctl/UniquePtr.h>

namespace ncine {

class AppConfiguration;

}

class Canvas;
class SpriteManager;
class AnimationManager;
class UserInterface;

namespace nc = ncine;

/// My nCine event handler
class MyEventHandler :
    public nc::IAppEventHandler,
    public nc::IInputEventHandler
{
  public:
	void onPreInit(nc::AppConfiguration &config) override;
	void onInit() override;
	void onShutdown() override;
	void onFrameStart() override;

	void onKeyPressed(const nc::KeyboardEvent &event) override;
	void onKeyReleased(const nc::KeyboardEvent &event) override;

  private:
	nctl::UniquePtr<Canvas> ca_;
	nctl::UniquePtr<SpriteManager> spriteMgr_;
	nctl::UniquePtr<AnimationManager> animMgr_;
	nctl::UniquePtr<UserInterface> ui_;
};

#endif
