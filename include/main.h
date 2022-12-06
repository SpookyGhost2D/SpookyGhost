#ifndef CLASS_MYEVENTHANDLER
#define CLASS_MYEVENTHANDLER

#include <ncine/IAppEventHandler.h>
#include <ncine/IInputEventHandler.h>
#include <nctl/UniquePtr.h>

namespace ncine {

class AppConfiguration;

}

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
	void onChangeScalingFactor(float factor) override;

	void onKeyPressed(const nc::KeyboardEvent &event) override;
	void onKeyReleased(const nc::KeyboardEvent &event) override;
	void onFilesDropped(const nc::DropEvent &event) override;
	bool onQuitRequest() override;

  private:
	nctl::UniquePtr<UserInterface> ui_;
};

#endif
