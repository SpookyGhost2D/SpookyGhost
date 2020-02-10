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
#include "GridFunctionLibrary.h"
#include "ParallelAnimationGroup.h"
#include "SequentialAnimationGroup.h"

#include <ncine/Application.h>
#include <ncine/IFile.h>

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
	config.vaoPoolSize = 1; // TODO: FIX size > 1

	config.windowTitle = "SpookyGhost";
	config.windowIconFilename = "icon96.png";

	config.consoleLogLevel = nc::ILogger::LogLevel::WARN;
}

void MyEventHandler::onInit()
{
	RenderingResources::create();
	GridFunctionLibrary::init();

	canvas_ = nctl::makeUnique<Canvas>(256, 256); // TODO: Hard-coded initial size
	resizedCanvas_ = nctl::makeUnique<Canvas>();
	spritesheet_ = nctl::makeUnique<Canvas>();
	spriteMgr_ = nctl::makeUnique<SpriteManager>();
	animMgr_ = nctl::makeUnique<AnimationManager>();

	ui_ = nctl::makeUnique<UserInterface>(*canvas_, *resizedCanvas_, *spritesheet_, *spriteMgr_, *animMgr_);
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

	const SaveAnim &saveAnimStatus = ui_->saveAnimStatus();
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
		else if (event.sym == nc::KeySym::Q)
			nc::theApplication().quit();
	}
	if (event.sym == nc::KeySym::DELETE)
		ui_->removeSelectedSpriteWithKey();
	else if (event.sym == nc::KeySym::RIGHT)
		ui_->moveSprite(1, 0);
	else if (event.sym == nc::KeySym::LEFT)
		ui_->moveSprite(-1, 0);
	else if (event.sym == nc::KeySym::DOWN)
		ui_->moveSprite(0, 1);
	else if (event.sym == nc::KeySym::UP)
		ui_->moveSprite(0, -1);
}

void MyEventHandler::onKeyReleased(const nc::KeyboardEvent &event)
{
	if (event.sym == nc::KeySym::ESCAPE)
	{
		ui_->closeAboutWindow();
		ui_->cancelRender();
	}
}
