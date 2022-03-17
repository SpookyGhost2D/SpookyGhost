set(NCPROJECT_NAME "SpookyGhost")
set(NCPROJECT_EXE_NAME "spookyghost")
set(NCPROJECT_VENDOR "Angelo Theodorou")
set(NCPROJECT_COPYRIGHT "Copyright ©2020-2022 ${NCPROJECT_VENDOR}")
set(NCPROJECT_DESCRIPTION "A procedural sprite animation tool")
set(NCPROJECT_HOMEPAGE "https://encelo.itch.io/spookyghost")
set(NCPROJECT_REVERSE_DNS "io.itch.encelo.spookyghost")
set(NCPROJECT_XDGDESKTOP_CATEGORIES "Graphics")
set(NCPROJECT_MACOS_CATEGORY "public.app-category.graphics-design")

set(NCPROJECT_INCLUDE_DIRS include)

set(NCPROJECT_SOURCES
	include/singletons.h
	include/main.h
	include/shader_strings.h
	include/Canvas.h
	include/Sprite.h
	include/Texture.h
	include/RenderingResources.h
	include/LoopComponent.h
	include/EasingCurve.h
	include/IAnimation.h
	include/CurveAnimation.h
	include/PropertyAnimation.h
	include/GridAnimation.h
	include/AnimationGroup.h
	include/SequentialAnimationGroup.h
	include/ParallelAnimationGroup.h
	include/AnimationManager.h
	include/SpriteManager.h
	include/GridFunction.h
	include/GridFunctionParameter.h
	include/GridFunctionLibrary.h
	include/LuaSerializer.h
	include/LuaSaver.h
	include/Serializers.h
	include/Script.h
	include/ScriptManager.h
	include/ScriptAnimation.h
	include/SpriteEntry.h

	include/gui/gui_labels.h
	include/gui/gui_tips.h
	include/gui/gui_common.h
	include/gui/UserInterface.h
	include/gui/CanvasGuiSection.h
	include/gui/RenderGuiWindow.h
	include/gui/FileDialog.h

	src/singletons.cpp
	src/main.cpp
	src/shader_strings.cpp
	src/Canvas.cpp
	src/Sprite.cpp
	src/Texture.cpp
	src/RenderingResources.cpp
	src/LoopComponent.cpp
	src/EasingCurve.cpp
	src/IAnimation.cpp
	src/CurveAnimation.cpp
	src/PropertyAnimation.cpp
	src/GridAnimation.cpp
	src/AnimationGroup.cpp
	src/SequentialAnimationGroup.cpp
	src/ParallelAnimationGroup.cpp
	src/AnimationManager.cpp
	src/SpriteManager.cpp
	src/GridFunction.cpp
	src/GridFunctionLibrary.cpp
	src/LuaSerializer.cpp
	src/LuaSaver.cpp
	src/Serializers.cpp
	src/Script.cpp
	src/ScriptManager.cpp
	src/ScriptAnimation.cpp
	src/SpriteEntry.cpp

	src/gui/gui_common.cpp
	src/gui/UserInterface.cpp
	src/gui/CanvasGuiSection.cpp
	src/gui/RenderGuiWindow.cpp
	src/gui/config_window.cpp
	src/gui/style.cpp
	src/gui/openfile.cpp
	src/gui/FileDialog.cpp
)

option(CUSTOM_ITCHIO_BUILD "Create a build for the Itch.io store" ON)
option(CUSTOM_WITH_FONTAWESOME "Download FontAwesome and include it in ImGui atlas" ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

function(callback_before_target)
	if(CUSTOM_ITCHIO_BUILD)
		if(NOT APPLE)
			install(FILES .itch.toml DESTINATION .)
		endif()
		if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
			install(FILES launch.sh
				PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE
				DESTINATION .)

			if(NCINE_WITH_GLEW)
				install_linux_library(GLEW::GLEW)
			endif()
			if(NCINE_WITH_SDL)
				install_linux_library(SDL2::SDL2)
			elseif(NCINE_WITH_GLFW)
				install_linux_library(GLFW::GLFW)
			endif()
			if(NCINE_WITH_PNG)
				install_linux_library(ZLIB::ZLIB)
				install_linux_library(PNG::PNG)
			endif()
			if(NCINE_WITH_WEBP)
				install_linux_library(WebP::WebP)
			endif()
			if(NCINE_WITH_LUA)
				install_linux_library(Lua::Lua)
			endif()
		endif()
	endif()

	if(NCPROJECT_OPTIONS_PRESETS STREQUAL "BinDist")
		set(CUSTOM_WITH_FONTAWESOME ON CACHE BOOL "Download FontAwesome and include it in ImGui atlas" FORCE)
	endif()
endfunction()

function(callback_after_target)
	if(WIN32)
		target_compile_definitions(${NCPROJECT_EXE_NAME} PRIVATE "WITH_GLEW")
		if(MSVC)
			target_include_directories(${NCPROJECT_EXE_NAME} PRIVATE ${EXTERNAL_MSVC_DIR}/include)
		endif()
	endif()

	include(custom_iconfontcppheaders)
	if(NOT CMAKE_SYSTEM_NAME STREQUAL "Android" AND IS_DIRECTORY ${NCPROJECT_DATA_DIR})
		set(PROJECTS_WILDCARD "${NCPROJECT_DATA_DIR}/data/projects/*.lua")
		set(TEXTURES_WILDCARD "${NCPROJECT_DATA_DIR}/data/*.png")
		set(SCRIPTS_WILDCARD "${NCPROJECT_DATA_DIR}/data/scripts/*.lua")

		generate_files_list(${PROJECTS_WILDCARD} "Projects")
		generate_files_list(${TEXTURES_WILDCARD} "Textures")
		generate_files_list(${SCRIPTS_WILDCARD} "Scripts")

		include(custom_fontawesome)
		if(CUSTOM_WITH_FONTAWESOME)
			file(GLOB FONT_FILES "${NCPROJECT_DATA_DIR}/data/fonts/*.ttf")
		endif()
		if(EXISTS ${NCPROJECT_DATA_DIR}/data/config.lua)
			set(CONFIG_FILE ${NCPROJECT_DATA_DIR}/data/config.lua)
		endif()
		file(GLOB PROJECT_FILES ${PROJECTS_WILDCARD})
		file(GLOB TEXTURE_FILES ${TEXTURES_WILDCARD})
		file(GLOB SCRIPT_FILES ${SCRIPTS_WILDCARD})
		set(NCPROJECT_ANDROID_ASSETS ${CONFIG_FILE} ${FONT_FILES} ${PROJECT_FILES} ${TEXTURE_FILES} ${SCRIPT_FILES} CACHE STRING "" FORCE)
	endif()

	if(CMAKE_SYSTEM_NAME STREQUAL "Android" AND CUSTOM_WITH_FONTAWESOME)
		target_compile_definitions(${NCPROJECT_EXE_NAME} PRIVATE "WITH_FONTAWESOME")
		target_include_directories(${NCPROJECT_EXE_NAME} PRIVATE ${NCPROJECT_BINARY_DIR}/${ICONFONTCPPHEADERS_SOURCE_DIR_NAME})
	endif()

	# Needed to compile on Android
	set(GENERATED_SOURCES ${GENERATED_SOURCES} PARENT_SCOPE)
endfunction()

function(callback_end)
	if(NOT CMAKE_SYSTEM_NAME STREQUAL "Android" AND IS_DIRECTORY ${NCPROJECT_DATA_DIR}/docs)
		get_filename_component(PARENT_DATA_INSTALL_DESTINATION ${DATA_INSTALL_DESTINATION} DIRECTORY)
		if(PARENT_DATA_INSTALL_DESTINATION STREQUAL "")
			set(PARENT_DATA_INSTALL_DESTINATION .)
		endif()
		if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
			install(DIRECTORY ${NCPROJECT_DATA_DIR}/docs/ DESTINATION ${README_INSTALL_DESTINATION} PATTERN "*.adoc" EXCLUDE)
		else()
			install(DIRECTORY ${NCPROJECT_DATA_DIR}/docs DESTINATION ${PARENT_DATA_INSTALL_DESTINATION} PATTERN "*.adoc" EXCLUDE)
		endif()
	endif()

	if(CUSTOM_ITCHIO_BUILD AND CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NCPROJECT_OPTIONS_PRESETS STREQUAL "BinDist")
		# Set a relative data directory when building for Itch.io on Linux
		file(RELATIVE_PATH NCPROJECT_DEFAULT_DATA_DIR_NEW
				${CMAKE_INSTALL_PREFIX}/${RUNTIME_INSTALL_DESTINATION}
				${CMAKE_INSTALL_PREFIX}/${DATA_INSTALL_DESTINATION}) # Always strips trailing slash
		set(NCPROJECT_DEFAULT_DATA_DIR_NEW "${NCPROJECT_DEFAULT_DATA_DIR_NEW}/")

		get_target_property(COMPILE_DEFS ${NCPROJECT_EXE_NAME} COMPILE_DEFINITIONS)
		string(REPLACE ${NCPROJECT_DEFAULT_DATA_DIR} ${NCPROJECT_DEFAULT_DATA_DIR_NEW} COMPILE_DEFS "${COMPILE_DEFS}")
		set_property(TARGET ${NCPROJECT_EXE_NAME} PROPERTY COMPILE_DEFINITIONS ${COMPILE_DEFS})
	endif()
endfunction()

function(generate_files_list WILDCARD TYPE)
	string(TOLOWER ${TYPE} TYPE_LOWER)
	string(TOUPPER ${TYPE} TYPE_UPPER)

	get_filename_component(FILES_DIR ${WILDCARD} DIRECTORY)
	get_filename_component(GLOB_PATTERN ${WILDCARD} NAME)

	set(NUM_FILES 0)
	if(IS_DIRECTORY ${FILES_DIR})
		file(GLOB FILE_LIST "${FILES_DIR}/${GLOB_PATTERN}")
		list(LENGTH FILE_LIST NUM_FILES)
	endif()

	set(STRINGS_H_FILE "${GENERATED_INCLUDE_DIR}/${TYPE_LOWER}_strings.h")
	set(STRINGS_CPP_FILE "${GENERATED_SOURCE_DIR}/${TYPE_LOWER}_strings.cpp")

	get_filename_component(STRINGS_H_FILENAME ${STRINGS_H_FILE} NAME)
	file(WRITE ${STRINGS_H_FILE} "#ifndef NCPROJECT_${TYPE_UPPER}_STRINGS\n")
	file(APPEND ${STRINGS_H_FILE} "#define NCPROJECT_${TYPE_UPPER}_STRINGS\n\n")
	file(APPEND ${STRINGS_H_FILE} "struct ${TYPE}Strings\n{\n")
	file(APPEND ${STRINGS_H_FILE} "\tstatic const int Count = ${NUM_FILES};\n")
	file(APPEND ${STRINGS_H_FILE} "\tstatic char const * const Names[Count];\n};\n\n")
	file(APPEND ${STRINGS_H_FILE} "#endif\n")

	file(WRITE ${STRINGS_CPP_FILE} "#include \"${STRINGS_H_FILENAME}\"\n\n")
	file(APPEND ${STRINGS_CPP_FILE} "char const * const ${TYPE}Strings::Names[${TYPE}Strings::Count] =\n")
	file(APPEND ${STRINGS_CPP_FILE} "{\n")
	foreach(SINGLE_FILE ${FILE_LIST})
		get_filename_component(SINGLE_FILENAME ${SINGLE_FILE} NAME)
		file(APPEND ${STRINGS_CPP_FILE} "\t\"${SINGLE_FILENAME}\",\n")
	endforeach()
	file(APPEND ${STRINGS_CPP_FILE} "};\n")

	target_sources(${NCPROJECT_EXE_NAME} PRIVATE ${STRINGS_H_FILE} ${STRINGS_CPP_FILE})
	list(APPEND GENERATED_SOURCES ${STRINGS_CPP_FILE})
	set(GENERATED_SOURCES ${GENERATED_SOURCES} PARENT_SCOPE)
endfunction()

function(install_linux_library TARGET_NAME)
	get_target_property(LIB_LOCATION ${TARGET_NAME} IMPORTED_LOCATION)
	if(NOT LIB_LOCATION)
		get_target_property(LIB_LOCATION ${TARGET_NAME} IMPORTED_LOCATION_RELEASE)
	endif()
	get_filename_component(LIB_LOCATION ${LIB_LOCATION} REALPATH) # resolve symlink
	install(FILES ${LIB_LOCATION} DESTINATION lib64)

	get_filename_component(LIB_NAME ${LIB_LOCATION} NAME)
	set(LIB_NAME_STRIPPED ${LIB_NAME})
	string(REGEX MATCH "\\.[0-9]+$" LIB_REGEX_MATCH ${LIB_NAME_STRIPPED})
	while(LIB_REGEX_MATCH)
		string(REGEX REPLACE "\\.[0-9]+$" "" LIB_NAME_STRIPPED ${LIB_NAME_STRIPPED}) # remove last version number
		install(CODE "file(CREATE_LINK \"${LIB_NAME}\" \"\${CMAKE_INSTALL_PREFIX}/lib64/${LIB_NAME_STRIPPED}\" SYMBOLIC)")
		string(REGEX MATCH "\\.[0-9]+$" LIB_REGEX_MATCH ${LIB_NAME_STRIPPED})
	endwhile()
endfunction()

# Don't edit beyond this line
if(CMAKE_SYSTEM_NAME STREQUAL "Android")
	return()
endif()

cmake_minimum_required(VERSION 3.13)
project(${NCPROJECT_NAME})

find_path(NCPROJECT_DIR cmake/project_main.cmake
	PATHS ${nCine_DIR} $ENV{NCINE_DIR}
	PATH_SUFFIXES project ../project ../../../project ../../../share/ncine/project
	NO_CMAKE_FIND_ROOT_PATH # For Emscripten
	DOC "Path to the nCine template project scripts")

if(IS_DIRECTORY ${NCPROJECT_DIR})
	include(${NCPROJECT_DIR}/cmake/project_main.cmake)
else()
	if(NOT DEFINED nCine_DIR AND "$ENV{NCINE_DIR}" STREQUAL "")
		message(FATAL_ERROR "Set the nCine_DIR CMake variable or the NCINE_DIR environment variable to a valid nCine directory.")
	elseif(DEFINED nCine_DIR)
		message(FATAL_ERROR "The nCine_DIR CMake variable \"${nCine_DIR}\" is not a valid nCine directory.")
	elseif(NOT "$ENV{NCINE_DIR}" STREQUAL "")
		message(FATAL_ERROR "The NCINE_DIR environment variable \"$ENV{NCINE_DIR}\" is not a valid nCine directory.")
	endif()
endif()
