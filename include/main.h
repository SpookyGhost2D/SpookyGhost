#ifndef CLASS_MYEVENTHANDLER
#define CLASS_MYEVENTHANDLER

#include <ncine/IAppEventHandler.h>
#include <ncine/IInputEventHandler.h>
#include <nctl/UniquePtr.h>

namespace ncine {

class AppConfiguration;

}

class Canvas;
class AnimationManager;
class UserInterface;
class Texture;
class Sprite;

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

	void onKeyReleased(const nc::KeyboardEvent &event) override;

  private:
	nctl::UniquePtr<Canvas> ca_;
	nctl::UniquePtr<AnimationManager> animMgr_;
	nctl::UniquePtr<UserInterface> ui_;

	nctl::UniquePtr<Texture> texture_;
	nctl::UniquePtr<Sprite> sprite_;
};

#endif
