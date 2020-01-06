#include "main.h"
#include "RenderResources.h"
#include "Canvas.h"
#include "UserInterface.h"

#include "AnimationManager.h"
#include "PropertyAnimation.h"
#include "ParallelAnimationGroup.h"
#include "SequentialAnimationGroup.h"

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

	ca_ = nctl::makeUnique<Canvas>(imageWidth, imageHeight);
	animMgr_ = nctl::makeUnique<AnimationManager>();

	texture_ = nctl::makeUnique<Texture>((nc::IFile::dataPath() + "dog.png").data());
	sprite_ = nctl::makeUnique<Sprite>(texture_.get());
	sprite_->x = 11.0f;//150.0f;
	sprite_->y = 100.0f;
	sprite_->rotation = 45.0f;

	ui_ = nctl::makeUnique<UserInterface>(*ca_, *animMgr_, *sprite_);

	nctl::UniquePtr<ParallelAnimationGroup> animGroup = nctl::makeUnique<ParallelAnimationGroup>();

	nctl::UniquePtr<PropertyAnimation> anim = nctl::makeUnique<PropertyAnimation>(EasingCurve::Type::QUAD, EasingCurve::LoopMode::PING_PONG);
	anim->curve().setScale(150.0f);
	anim->setProperty(&sprite_->y);
	anim->setPropertyName("Position Y");
	animGroup->anims().pushBack(nctl::move(anim));

	anim = nctl::makeUnique<PropertyAnimation>(EasingCurve::Type::QUAD, EasingCurve::LoopMode::PING_PONG);
	anim->curve().setScale(150.0f);
	anim->setProperty(&sprite_->rotation);
	anim->setPropertyName("Rotation");
	animGroup->anims().pushBack(nctl::move(anim));

	anim = nctl::makeUnique<PropertyAnimation>(EasingCurve::Type::QUAD, EasingCurve::LoopMode::PING_PONG);
	anim->curve().setScale(20.0f);
	anim->setProperty(&sprite_->x);
	anim->setPropertyName("Position X");
	animGroup->anims().pushBack(nctl::move(anim));

	animGroup->play();
	animMgr_->anims().pushBack(nctl::move(animGroup));
}

void MyEventHandler::onShutdown()
{
	RenderResources::dispose();
}

void MyEventHandler::onFrameStart()
{
	const float interval = nc::theApplication().interval();
	glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	ca_->bind();

	static float angle = 0.0f;
	angle += 5.0f * interval;
	if (ui_->shouldSaveAnim() == false)
		animMgr_->update(interval);

	sprite_->testAnim(angle);
	sprite_->transform();
	sprite_->updateRender();
	sprite_->render();
	ca_->unbind();

	ui_->createGui();

	if (ui_->shouldSaveAnim())
	{
		const UserInterface::SaveAnim saveAnimStatus = ui_->saveAnimStatus();
		if (saveAnimStatus.numSavedFrames == 0)
			animMgr_->reset();

			ca_->save(saveAnimStatus.filename.data());
			ui_->signalFrameSaved();
			animMgr_->update(saveAnimStatus.inverseFps());
	}
}

void MyEventHandler::onKeyReleased(const nc::KeyboardEvent &event)
{
	if (event.sym == nc::KeySym::ESCAPE || event.sym == nc::KeySym::Q)
		nc::theApplication().quit();
}
