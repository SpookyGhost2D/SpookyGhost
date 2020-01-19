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

	config.windowTitle = "SpookyGhost";
	config.windowIconFilename = "icon84.png";

	config.consoleLogLevel = nc::ILogger::LogLevel::WARN;
}

void MyEventHandler::onInit()
{
	RenderingResources::create();

	canvas_ = nctl::makeUnique<Canvas>(imageWidth, imageHeight);
	resizedCanvas_ = nctl::makeUnique<Canvas>();
	spritesheet_ = nctl::makeUnique<Canvas>();
	spriteMgr_ = nctl::makeUnique<SpriteManager>();
	animMgr_ = nctl::makeUnique<AnimationManager>();

	nctl::UniquePtr<Texture> texture = nctl::makeUnique<Texture>((nc::IFile::dataPath() + "icon84.png").data());
	nctl::UniquePtr<Sprite> sprite = nctl::makeUnique<Sprite>(texture.get());
	sprite->name = "Ghost";
	sprite->x = 100.0f;
	sprite->y = 100.0f;

	nctl::UniquePtr<Texture> texture2 = nctl::makeUnique<Texture>((nc::IFile::dataPath() + "ncine84.png").data());
	nctl::UniquePtr<Sprite> sprite2 = nctl::makeUnique<Sprite>(texture2.get());
	sprite2->name = "nCine";
	sprite2->x = 100.0f;
	sprite2->y = 100.0f;

	ui_ = nctl::makeUnique<UserInterface>(*canvas_, *resizedCanvas_, *spritesheet_, *spriteMgr_, *animMgr_);

	nctl::UniquePtr<ParallelAnimationGroup> animGroup = nctl::makeUnique<ParallelAnimationGroup>();

	nctl::UniquePtr<GridAnimation> gridAnim = nctl::makeUnique<GridAnimation>(EasingCurve::Type::LINEAR, EasingCurve::LoopMode::PING_PONG);
	gridAnim->curve().setScale(10.0f);
	gridAnim->setSprite(sprite.get());
	gridAnim->setGridAnimationType(GridAnimation::AnimationType::WOBBLE_X);
	animGroup->anims().pushBack(nctl::move(gridAnim));

	nctl::UniquePtr<PropertyAnimation> anim = nctl::makeUnique<PropertyAnimation>(EasingCurve::Type::QUAD, EasingCurve::LoopMode::PING_PONG);
	anim->curve().setShift(sprite2->width() * 0.5f);
	anim->curve().setScale(20.0f);
	anim->setProperty(&sprite2->x);
	anim->setPropertyName("Position X");
	anim->setSprite(sprite2.get());
	animGroup->anims().pushBack(nctl::move(anim));

	anim = nctl::makeUnique<PropertyAnimation>(EasingCurve::Type::QUAD, EasingCurve::LoopMode::PING_PONG);
	anim->curve().setShift(sprite2->height() * 0.5f);
	anim->curve().setScale(150.0f);
	anim->setProperty(&sprite2->y);
	anim->setPropertyName("Position Y");
	anim->setSprite(sprite2.get());
	animGroup->anims().pushBack(nctl::move(anim));

	anim = nctl::makeUnique<PropertyAnimation>(EasingCurve::Type::QUAD, EasingCurve::LoopMode::PING_PONG);
	anim->curve().setScale(150.0f);
	anim->setProperty(&sprite2->rotation);
	anim->setPropertyName("Rotation");
	anim->setSprite(sprite2.get());
	animGroup->anims().pushBack(nctl::move(anim));

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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	canvas_->bind();

	const UserInterface::SaveAnim &saveAnimStatus = ui_->saveAnimStatus();
	if (ui_->shouldSaveFrames() || ui_->shouldSaveSpritesheet())
	{
		if (saveAnimStatus.numSavedFrames == 0)
		{
			animMgr_->reset();
			animMgr_->update(0.0f);
		}
	}
	else
		animMgr_->update(interval);
	spriteMgr_->update();

	canvas_->unbind();

	if (ui_->shouldSaveFrames() || ui_->shouldSaveSpritesheet())
	{
		Canvas *sourceCanvas = (saveAnimStatus.canvasResize != 1.0f) ? resizedCanvas_.get() : canvas_.get();
		if (saveAnimStatus.canvasResize != 1.0f)
		{
			canvas_->bindRead();
			resizedCanvas_->bindDraw();
			glBlitFramebuffer(0, 0, canvas_->texWidth(), canvas_->texHeight(), 0, 0, resizedCanvas_->texWidth(), resizedCanvas_->texHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
			canvas_->unbind();
			resizedCanvas_->unbind();
		}

		if (ui_->shouldSaveFrames())
			sourceCanvas->save(saveAnimStatus.filename.data());
		else if (ui_->shouldSaveSpritesheet())
		{
			sourceCanvas->bindRead();
			spritesheet_->bindTexture();
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, saveAnimStatus.sheetDestPos.x, saveAnimStatus.sheetDestPos.y, 0, 0, sourceCanvas->texWidth(), sourceCanvas->texHeight());
			sourceCanvas->unbind();
			spritesheet_->unbindTexture();
		}

		const bool shouldSaveSpritesheet = ui_->shouldSaveSpritesheet();
		ui_->signalFrameSaved();
		// If this was the last frame to blit then we save the spritesheet
		if (shouldSaveSpritesheet && ui_->shouldSaveSpritesheet() == false)
			spritesheet_->save(saveAnimStatus.filename.data());
		animMgr_->update(saveAnimStatus.inverseFps());
	}

	ui_->createGui();
}

void MyEventHandler::onKeyPressed(const nc::KeyboardEvent &event)
{
	if (event.mod & nc::KeyMod::CTRL)
	{
		if (event.sym == nc::KeySym::N)
			ui_->menuNew();
		if (event.sym == nc::KeySym::Q)
			nc::theApplication().quit();
	}
}

void MyEventHandler::onKeyReleased(const nc::KeyboardEvent &event)
{
	if (event.sym == nc::KeySym::ESCAPE)
	{
		ui_->closeAboutWindow();
		ui_->cancelRender();
	}
}
