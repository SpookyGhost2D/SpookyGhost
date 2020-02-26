#ifndef SINGLETONS_H
#define SINGLETONS_H

#include <nctl/UniquePtr.h>
#include "Configuration.h"

class Canvas;
class SpriteManager;
class AnimationManager;
class UserInterface;
class LuaSaver;

extern Configuration theCfg;
extern nctl::UniquePtr<Canvas> theCanvas;
extern nctl::UniquePtr<Canvas> theResizedCanvas;
extern nctl::UniquePtr<Canvas> theSpritesheet;
extern nctl::UniquePtr<SpriteManager> theSpriteMgr;
extern nctl::UniquePtr<AnimationManager> theAnimMgr;
extern nctl::UniquePtr<LuaSaver> theSaver;

#endif
