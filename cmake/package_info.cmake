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
	include/gui_labels.h
	include/Canvas.h
	include/Sprite.h
	include/Texture.h
	include/UserInterface.h
	include/RenderResources.h
	include/EasingCurve.h
	include/IAnimation.h
	include/PropertyAnimation.h
	include/GridAnimation.h
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
	src/GridAnimation.cpp
	src/AnimationGroup.cpp
	src/SequentialAnimationGroup.cpp
	src/ParallelAnimationGroup.cpp
	src/AnimationManager.cpp
	#src/LuaSerializer.cpp
)

function(callback_before_target)
	option(CUSTOM_WITH_FONTAWESOME "Download FontAwesome and include it in ImGui atlas" ON)
	if(PACKAGE_OPTIONS_PRESETS STREQUAL BinDist)
		set(CUSTOM_WITH_FONTAWESOME ON CACHE BOOL "Download FontAwesome and include it in ImGui atlas" FORCE)
	endif()
endfunction()

function(callback_after_target)
	if(NOT CMAKE_SYSTEM_NAME STREQUAL "Android" AND IS_DIRECTORY ${PACKAGE_DATA_DIR})
		include(custom_fontawesome)
		include(custom_iconfontcppheaders)

		if(CUSTOM_WITH_FONTAWESOME)
			file(GLOB FONT_FILES "${PACKAGE_DATA_DIR}/data/fonts/*.ttf")
		endif()
	endif()

	if(CMAKE_SYSTEM_NAME STREQUAL "Android" AND CUSTOM_WITH_FONTAWESOME)
		target_compile_definitions(${PACKAGE_EXE_NAME} PRIVATE "WITH_FONTAWESOME")
		target_include_directories(${PACKAGE_EXE_NAME} PRIVATE ${GENERATED_INCLUDE_DIR}/../../iconfontcppheaders-src)
	endif()

	# Needed to compile on Android
	set(GENERATED_SOURCES ${GENERATED_SOURCES} PARENT_SCOPE)
endfunction()
