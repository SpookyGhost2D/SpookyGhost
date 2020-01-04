#define NCINE_INCLUDE_OPENGL
#include <ncine/common_headers.h>

#include "main.h"
#include "RenderResources.h"
#include "Canvas.h"
#include "UserInterface.h"

#include "AnimationManager.h"

#include <ncine/Application.h>
#include <ncine/IFile.h>

#include "Texture.h"
#include "Sprite.h"

namespace {

const unsigned int imageWidth = 256;
const unsigned int imageHeight = 256;

}

nc::IAppEventHandler *createAppEventHandler()
{
	return new MyEventHandler;
}

void MyEventHandler::onPreInit(nc::AppConfiguration &config)
{
#if defined(__ANDROID__)
    config.dataPath() = "asset::";
#elif defined(__EMSCRIPTEN__)
    config.dataPath() = "/";
#else
    #ifdef PACKAGE_DEFAULT_DATA_DIR
    config.dataPath() = PACKAGE_DEFAULT_DATA_DIR;
    #else
    config.dataPath() = "data/";
    #endif
#endif

	config.resolution.x = 1280;
	config.resolution.y = 720;

	config.withScenegraph = false;
	config.withAudio = false;
	config.withDebugOverlay = false;
	config.withThreads = false;
	config.isResizable = true;
	//config.hasBorders = false;
	config.vaoPoolSize = 1; // TODO: FIX size > 1

	config.windowTitle = "ncSpookyGhost";
	config.windowIconFilename = "icon48.png";

	config.consoleLogLevel = nc::ILogger::LogLevel::WARN;
}

void MyEventHandler::onInit()
{
	RenderResources::create();

	ca_ = nctl::makeUnique<Canvas>();
	ca_->initTexture(imageWidth, imageHeight);
	animMgr_ = nctl::makeUnique<AnimationManager>();
	ui_ = nctl::makeUnique<UserInterface>(*ca_, *animMgr_);

	texture_ = nctl::makeUnique<Texture>((nc::IFile::dataPath() + "dog.png").data());
	sprite_ = nctl::makeUnique<Sprite>(texture_.get());
	sprite_->x = 11.0f;//150.0f;
	sprite_->y = 100.0f;
	sprite_->rotation = 45.0f;
	saveAnim_ = false;

	Animation anim(EasingCurve::Type::QUAD, EasingCurve::LoopMode::PING_PONG);
	anim.curve().setCoefficients(150.0f, 0.0f, 0.0f);
	anim.setProperty(&sprite_->y);
	anim.play();
	animMgr_->anims().pushBack(anim);

	Animation anim2(EasingCurve::Type::QUAD, EasingCurve::LoopMode::PING_PONG);
	anim2.curve().setCoefficients(150.0f, 0.0f, 0.0f);
	anim2.setProperty(&sprite_->rotation);
	anim2.play();
	animMgr_->anims().pushBack(anim2);

	Animation anim3(EasingCurve::Type::QUAD, EasingCurve::LoopMode::PING_PONG);
	anim3.curve().setCoefficients(20.0f, 0.0f, 0.0f);
	anim3.setProperty(&sprite_->x);
	anim3.play();
	animMgr_->anims().pushBack(anim3);
}

void MyEventHandler::onShutdown()
{
	RenderResources::dispose();
}

void MyEventHandler::onFrameStart()
{
	const float interval = saveAnim_ ? 1.0f / 60.0f : nc::theApplication().interval();
	glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	ca_->bind();

	static float angle = 0.0f;
	angle += 5.0f * interval;
	animMgr_->update(interval);

	sprite_->testAnim(angle);
	sprite_->transform();
	sprite_->updateRender();
	sprite_->render();
	ca_->unbind();

	ui_->createGuiMainWindow();

	if (saveAnim_)
	{
		if (currentFrame_ > 60)
			saveAnim_ = false;
		else if (currentFrame_ > 0)
		{
			nctl::String frameName(32);
			frameName.format("test_%03d.png", currentFrame_);
			ca_->save(frameName.data());
		}
		currentFrame_++;
	}
}

void MyEventHandler::onKeyReleased(const nc::KeyboardEvent &event)
{
	if (event.sym == nc::KeySym::ESCAPE || event.sym == nc::KeySym::Q)
		nc::theApplication().quit();
	if (event.sym == nc::KeySym::SPACE)
	{
		currentFrame_ = 0;
		saveAnim_ = true;
		animMgr_->reset();
	}
}
