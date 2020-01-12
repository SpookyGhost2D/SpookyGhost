#define NCINE_INCLUDE_OPENGL
#include <ncine/common_headers.h>

#include "main.h"
#include "RenderingResources.h"
#include "Canvas.h"
#include "SpriteManager.h"
#include "UserInterface.h"

#include "AnimationManager.h"
#include "PropertyAnimation.h"
#include "GridAnimation.h"
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
	RenderingResources::create();

	ca_ = nctl::makeUnique<Canvas>(imageWidth, imageHeight);
	spriteMgr_ = nctl::makeUnique<SpriteManager>();
	animMgr_ = nctl::makeUnique<AnimationManager>();

	nctl::UniquePtr<Texture> texture = nctl::makeUnique<Texture>((nc::IFile::dataPath() + "icon48.png").data());
	nctl::UniquePtr<Sprite> sprite = nctl::makeUnique<Sprite>(texture.get());
	sprite->name = "Ghost";
	sprite->x = 50.0f;
	sprite->y = 100.0f;

	nctl::UniquePtr<Texture> texture2 = nctl::makeUnique<Texture>((nc::IFile::dataPath() + "dog.png").data());
	nctl::UniquePtr<Sprite> sprite2 = nctl::makeUnique<Sprite>(texture2.get());
	sprite2->name = "Dog";
	sprite2->x = 100.0f;
	sprite2->y = 100.0f;

	ui_ = nctl::makeUnique<UserInterface>(*ca_, *spriteMgr_, *animMgr_);

	nctl::UniquePtr<ParallelAnimationGroup> animGroup = nctl::makeUnique<ParallelAnimationGroup>();

#if 0
	nctl::UniquePtr<PropertyAnimation> anim = nctl::makeUnique<PropertyAnimation>(EasingCurve::Type::QUAD, EasingCurve::LoopMode::PING_PONG);
	anim->curve().setScale(20.0f);
	anim->setProperty(&sprite_->x);
	anim->setPropertyName("Position X");
	animGroup->anims().pushBack(nctl::move(anim));

	anim = nctl::makeUnique<PropertyAnimation>(EasingCurve::Type::QUAD, EasingCurve::LoopMode::PING_PONG);
	anim->curve().setScale(150.0f);
	anim->setProperty(&sprite_->y);
	anim->setPropertyName("Position Y");
	animGroup->anims().pushBack(nctl::move(anim));

	anim = nctl::makeUnique<PropertyAnimation>(EasingCurve::Type::QUAD, EasingCurve::LoopMode::PING_PONG);
	anim->curve().setScale(150.0f);
	anim->setProperty(&sprite_->rotation);
	anim->setPropertyName("Rotation");
	animGroup->anims().pushBack(nctl::move(anim));
#endif
	nctl::UniquePtr<GridAnimation> gridAnim = nctl::makeUnique<GridAnimation>(EasingCurve::Type::LINEAR, EasingCurve::LoopMode::PING_PONG);
	gridAnim->curve().setScale(20.0f);
	gridAnim->setSprite(sprite.get());
	gridAnim->setGridAnimationType(GridAnimation::AnimationType::WOBBLE_Y);
	animGroup->anims().pushBack(nctl::move(gridAnim));

	animGroup->play();
	animMgr_->anims().pushBack(nctl::move(animGroup));

	spriteMgr_->textures().pushBack(nctl::move(texture));
	spriteMgr_->sprites().pushBack(nctl::move(sprite));

	spriteMgr_->textures().pushBack(nctl::move(texture2));
	spriteMgr_->sprites().pushBack(nctl::move(sprite2));
}

void MyEventHandler::onShutdown()
{
	RenderingResources::dispose();
}

void MyEventHandler::onFrameStart()
{
	const float interval = nc::theApplication().interval();
	glClearColor(0.25f, 0.25f, 0.25f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	ca_->bind();

	if (ui_->shouldSaveAnim() == false)
		animMgr_->update(interval);
	spriteMgr_->update();

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

void MyEventHandler::onKeyPressed(const nc::KeyboardEvent &event)
{
	if (event.mod & nc::KeyMod::CTRL)
	{
		if (event.sym == nc::KeySym::Q)
			nc::theApplication().quit();
	}
}

void MyEventHandler::onKeyReleased(const nc::KeyboardEvent &event)
{
	if (event.sym == nc::KeySym::ESCAPE)
		ui_->closeAboutWindow();
}
