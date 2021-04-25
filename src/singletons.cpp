#include "singletons.h"

Configuration theCfg;
nctl::UniquePtr<Canvas> theCanvas;
nctl::UniquePtr<Canvas> theResizedCanvas;
nctl::UniquePtr<Canvas> theSpritesheet;
nctl::UniquePtr<SpriteManager> theSpriteMgr;
nctl::UniquePtr<AnimationManager> theAnimMgr;
nctl::UniquePtr<LuaSaver> theSaver;
nctl::UniquePtr<ScriptManager> theScriptingMgr;
