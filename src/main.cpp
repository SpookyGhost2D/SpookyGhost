#include <ncine/config.h>
#if !NCINE_WITH_IMGUI
	#error nCine must have ImGui integration enabled for this application to work
#endif
#if !NCINE_WITH_LUA
	#error nCine must have Lua integration enabled for this application to work
#endif

#define NCINE_INCLUDE_OPENGL
#include <ncine/common_headers.h>
#ifdef __MINGW32__
	#undef ERROR
	#undef DELETE
	#undef far
	#undef near
#endif

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
#include "ScriptManager.h"

#include <ncine/Application.h>
#include <ncine/FileSystem.h>
#include <ncine/IFile.h>

#ifdef __ANDROID__
	#include <ncine/AndroidApplication.h>
#endif

nctl::UniquePtr<nc::IAppEventHandler> createAppEventHandler()
{
	return nctl::makeUnique<MyEventHandler>();
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

	ui::auxString = "config.lua";
#ifdef __ANDROID__
	ui::androidCfgDir = static_cast<nc::AndroidApplication &>(nc::theApplication()).internalDataPath();
	// TODO: Load textures from media directory and save projects and scripts in an external path
	ui::androidSaveDir = static_cast<nc::AndroidApplication &>(nc::theApplication()).internalDataPath();

	if (nc::fs::isDirectory(ui::androidSaveDir.data()) == false)
	{
		const bool dirCreated = nc::fs::createDir(ui::androidSaveDir.data());
		LOGI_X("Android save directory \"%s\" created: %s", ui::androidSaveDir.data(), dirCreated ? "true" : "false");
	}
	ui::auxString = nc::fs::joinPath(ui::androidSaveDir.data(), "projects");
	if (nc::fs::isDirectory(ui::auxString.data()) == false)
	{
		const bool dirCreated = nc::fs::createDir(ui::auxString.data());
		LOGI_X("Android projects directory \"%s\" created: %s", ui::auxString.data(), dirCreated ? "true" : "false");
	}
	ui::auxString = nc::fs::joinPath(ui::androidSaveDir.data(), "scripts");
	if (nc::fs::isDirectory(ui::auxString.data()) == false)
	{
		const bool dirCreated = nc::fs::createDir(ui::auxString.data());
		LOGI_X("Android scripts directory \"%s\" created: %s", ui::auxString.data(), dirCreated ? "true" : "false");
	}

	ui::auxString = nc::fs::joinPath(ui::androidCfgDir.data(), "config.lua");
	if (nc::fs::isReadableFile(ui::auxString.data()) == false)
		ui::auxString = "asset::config.lua";
#endif

	if (nc::fs::isReadableFile(ui::auxString.data()))
	{
		LuaSaver saver(4096);
		saver.loadCfg(ui::auxString.data(), theCfg);
	}

#ifdef __ANDROID__
	// Setting the default configuration directories
	if (nc::fs::isDirectory(theCfg.projectsPath.data()) == false)
		theCfg.projectsPath = nc::fs::joinPath(ui::androidSaveDir, "projects");
	if (nc::fs::isDirectory(theCfg.texturesPath.data()) == false)
		theCfg.texturesPath = ui::androidSaveDir;
	if (nc::fs::isDirectory(theCfg.scriptsPath.data()) == false)
		theCfg.scriptsPath = nc::fs::joinPath(ui::androidSaveDir, "scripts");

	// Setting the default data directories
	ui::projectsDataDir = "asset::projects";
	ui::texturesDataDir = "asset::";
	ui::scriptsDataDir = "asset::scripts";
#else
	// Setting the default configuration directories
	if (nc::fs::isDirectory(theCfg.projectsPath.data()) == false)
		theCfg.projectsPath = nc::fs::joinPath(nc::fs::dataPath(), "projects");
	if (nc::fs::isDirectory(theCfg.texturesPath.data()) == false)
		theCfg.texturesPath = nc::fs::dataPath();
	if (nc::fs::isDirectory(theCfg.scriptsPath.data()) == false)
		theCfg.scriptsPath = nc::fs::joinPath(nc::fs::dataPath(), "scripts");

	// Setting the default data directories
	ui::projectsDataDir = nc::fs::joinPath(nc::fs::dataPath(), "projects");
	ui::texturesDataDir = nc::fs::dataPath();
	ui::scriptsDataDir = nc::fs::joinPath(nc::fs::dataPath(), "scripts");
#endif

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
	theScriptingMgr = nctl::makeUnique<ScriptManager>();

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
			// Reset to initial state before playing
			theAnimMgr->stop();
			theAnimMgr->play();
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

		const bool shouldSaveBefore = (ui_->shouldSaveFrames() || ui_->shouldSaveSpritesheet());
		const bool shouldSaveSpritesheetBefore = ui_->shouldSaveSpritesheet();
		ui_->signalFrameSaved();
		const bool shouldSaveAfter = (ui_->shouldSaveFrames() || ui_->shouldSaveSpritesheet());
		// Check if this was the last frame
		if (shouldSaveBefore != shouldSaveAfter)
		{
			if (shouldSaveSpritesheetBefore)
				theSpritesheet->save(saveAnimStatus.filename.data());
			// Stop animations after the saving process is complete
			theAnimMgr->stop();
		}
		else
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
		else if (event.sym == nc::KeySym::R)
			ui_->reloadScript();
		else if (event.sym == nc::KeySym::Q)
			ui_->quit();
	}

	if (event.sym == nc::KeySym::F1 && ui_->openDocumentationEnabled())
		ui_->openDocumentation();
	if (event.sym == nc::KeySym::F5)
		ui_->menuQuickSave();
	if (event.sym == nc::KeySym::F9)
		ui_->menuQuickOpen();
	if (event.sym == nc::KeySym::SPACE)
		ui_->toggleAnimation();
	else if (event.sym == nc::KeySym::DELETE)
		ui_->pressDeleteKey();
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
		ui_->closeModalsAndUndockables();
		ui_->cancelRender();
	}
}

bool MyEventHandler::onQuitRequest()
{
	ui_->quit();
	// Ignore the quit request
	return false;
}
