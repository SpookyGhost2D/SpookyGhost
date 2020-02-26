#define NCINE_INCLUDE_OPENGL
#include <ncine/common_headers.h>

#include "singletons.h"
#include "main.h"
#include "RenderingResources.h"
#include "Canvas.h"
#include "SpriteManager.h"
#include "gui/UserInterface.h"

#include "AnimationManager.h"
#include "PropertyAnimation.h"
#include "GridAnimation.h"
#include "GridFunctionLibrary.h"
#include "ParallelAnimationGroup.h"
#include "SequentialAnimationGroup.h"
#include "LuaSaver.h"

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

	if (nc::IFile::access("config.lua", nc::IFile::AccessMode::READABLE))
	{
		LuaSaver saver(4096);
		saver.loadCfg("config.lua", theCfg);
	}

	if (nc::IFile::access(theCfg.scriptsPath.data(), nc::IFile::AccessMode::READABLE) == false)
		theCfg.scriptsPath = nc::IFile::dataPath() + "scripts/";
	if (nc::IFile::access(theCfg.texturesPath.data(), nc::IFile::AccessMode::READABLE) == false)
		theCfg.texturesPath = nc::IFile::dataPath();

	config.resolution.set(theCfg.width, theCfg.height);
	config.inFullscreen = theCfg.fullscreen;
	config.isResizable = theCfg.resizable;
	config.frameLimit = static_cast<unsigned int>(theCfg.frameLimit);
	config.withVSync = theCfg.withVSync;

	config.withScenegraph = false;
	config.withAudio = false;
	config.withDebugOverlay = false;
	config.withThreads = false;
	config.vaoPoolSize = 1; // TODO: FIX size > 1

	config.windowTitle = "SpookyGhost";
	config.windowIconFilename = "icon96.png";
}

void MyEventHandler::onInit()
{
	RenderingResources::create();
	GridFunctionLibrary::init();

	theCanvas = nctl::makeUnique<Canvas>(theCfg.canvasWidth, theCfg.canvasHeight);
	theResizedCanvas = nctl::makeUnique<Canvas>();
	theSpritesheet = nctl::makeUnique<Canvas>();
	theSpriteMgr = nctl::makeUnique<SpriteManager>();
	theAnimMgr = nctl::makeUnique<AnimationManager>();
	theSaver = nctl::makeUnique<LuaSaver>(theCfg.saveFileMaxSize);

	ui_ = nctl::makeUnique<UserInterface>();
}

void MyEventHandler::onShutdown()
{
	RenderingResources::dispose();
}

void MyEventHandler::onFrameStart()
{
	const float interval = nc::theApplication().interval();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	theCanvas->bind();

	const SaveAnim &saveAnimStatus = ui_->saveAnimStatus();
	if (ui_->shouldSaveFrames() || ui_->shouldSaveSpritesheet())
	{
		if (saveAnimStatus.numSavedFrames == 0)
		{
			theAnimMgr->reset();
			theAnimMgr->update(0.0f);
		}
	}
	else
		theAnimMgr->update(interval);
	theSpriteMgr->update();

	theCanvas->unbind();

	if (ui_->shouldSaveFrames() || ui_->shouldSaveSpritesheet())
	{
		Canvas *sourceCanvas = (saveAnimStatus.canvasResize != 1.0f) ? theResizedCanvas.get() : theCanvas.get();
		if (saveAnimStatus.canvasResize != 1.0f)
		{
			theCanvas->bindRead();
			theResizedCanvas->bindDraw();
			glBlitFramebuffer(0, 0, theCanvas->texWidth(), theCanvas->texHeight(), 0, 0, theResizedCanvas->texWidth(), theResizedCanvas->texHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
			theCanvas->unbind();
			theResizedCanvas->unbind();
		}

		if (ui_->shouldSaveFrames())
			sourceCanvas->save(saveAnimStatus.filename.data());
		else if (ui_->shouldSaveSpritesheet())
		{
			sourceCanvas->bindRead();
			theSpritesheet->bindTexture();
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, saveAnimStatus.sheetDestPos.x, saveAnimStatus.sheetDestPos.y, 0, 0, sourceCanvas->texWidth(), sourceCanvas->texHeight());
			sourceCanvas->unbind();
			theSpritesheet->unbindTexture();
		}

		const bool shouldSaveSpritesheet = ui_->shouldSaveSpritesheet();
		ui_->signalFrameSaved();
		// If this was the last frame to blit then we save the spritesheet
		if (shouldSaveSpritesheet && ui_->shouldSaveSpritesheet() == false)
			theSpritesheet->save(saveAnimStatus.filename.data());
		theAnimMgr->update(saveAnimStatus.inverseFps());
	}

	ui_->createGui();
}

void MyEventHandler::onKeyPressed(const nc::KeyboardEvent &event)
{
	if (event.mod & nc::KeyMod::CTRL)
	{
		if (event.sym == nc::KeySym::N && ui_->menuNewEnabled())
			ui_->menuNew();
		else if (event.sym == nc::KeySym::O)
			ui_->menuOpen();
		else if (event.sym == nc::KeySym::S && ui_->menuSaveEnabled())
			ui_->menuSave();
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
		ui_->closeModalsAndAbout();
		ui_->cancelRender();
	}
}
