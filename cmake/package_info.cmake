set(PACKAGE_NAME "ncSpookyGhost")
set(PACKAGE_EXE_NAME "ncspookyghost")
set(PACKAGE_VENDOR "Angelo Theodorou")
set(PACKAGE_COPYRIGHT "Copyright ©2020 ${PACKAGE_VENDOR}")
set(PACKAGE_DESCRIPTION "A procedural sprite animation tool")
set(PACKAGE_HOMEPAGE "https://ncine.github.io")
set(PACKAGE_REVERSE_DNS "io.github.ncine.spookyghost")

set(PACKAGE_INCLUDE_DIRS include)

set(PACKAGE_SOURCES
	include/main.h
	include/shader_strings.h
	include/Canvas.h
	include/Sprite.h
	include/Texture.h
	include/UserInterface.h
	include/RenderResources.h
	include/EasingCurve.h
	include/IAnimation.h
	include/PropertyAnimation.h
	include/AnimationGroup.h
	include/SequentialAnimationGroup.h
	include/ParallelAnimationGroup.h
	include/AnimationManager.h
	#include/LuaSerializer.h

	src/main.cpp
	src/shader_strings.cpp
	src/Canvas.cpp
	src/Sprite.cpp
	src/Texture.cpp
	src/UserInterface.cpp
	src/RenderResources.cpp
	src/EasingCurve.cpp
	src/PropertyAnimation.cpp
	src/AnimationGroup.cpp
	src/SequentialAnimationGroup.cpp
	src/ParallelAnimationGroup.cpp
	src/AnimationManager.cpp
	#src/LuaSerializer.cpp
)
