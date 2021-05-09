set(PACKAGE_NAME "SpookyGhost")
set(PACKAGE_EXE_NAME "spookyghost")
set(PACKAGE_VENDOR "Angelo Theodorou")
set(PACKAGE_COPYRIGHT "Copyright ©2020-2021 ${PACKAGE_VENDOR}")
set(PACKAGE_DESCRIPTION "A procedural sprite animation tool")
set(PACKAGE_HOMEPAGE "https://encelo.itch.io/spookyghost")
set(PACKAGE_REVERSE_DNS "io.itch.encelo.spookyghost")

set(PACKAGE_INCLUDE_DIRS include)

set(PACKAGE_SOURCES
	include/singletons.h
	include/main.h
	include/shader_strings.h
	include/Canvas.h
	include/Sprite.h
	include/Texture.h
	include/RenderingResources.h
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
	src/EasingCurve.cpp
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
option(CUSTOM_DEMO_VERSION "Create a build for the demo version" OFF)

function(callback_before_target)
	if(CUSTOM_ITCHIO_BUILD)
		if(NOT APPLE)
			install(FILES .itch.toml DESTINATION .)
		endif()
		if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
			install(FILES launch.sh
				PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE
				DESTINATION .)
		endif()
	endif()

	if(PACKAGE_OPTIONS_PRESETS STREQUAL "BinDist")
		set(CUSTOM_WITH_FONTAWESOME ON CACHE BOOL "Download FontAwesome and include it in ImGui atlas" FORCE)
	endif()
endfunction()

function(callback_after_target)
	if(CUSTOM_DEMO_VERSION)
		message(STATUS "Building the DEMO VERSION of SpookyGhost")
		target_compile_definitions(${PACKAGE_EXE_NAME} PRIVATE "DEMO_VERSION")
		set(PACKAGE_NAME "${PACKAGE_NAME}-Demo" PARENT_SCOPE)
	endif()

	if(MSVC)
		target_compile_definitions(${PACKAGE_EXE_NAME} PRIVATE "WITH_GLEW")
		target_include_directories(${PACKAGE_EXE_NAME} PRIVATE ${EXTERNAL_MSVC_DIR}/include)
	endif()

	include(custom_iconfontcppheaders)
	if(NOT CMAKE_SYSTEM_NAME STREQUAL "Android" AND IS_DIRECTORY ${PACKAGE_DATA_DIR})
		set(PROJECTS_WILDCARD "${PACKAGE_DATA_DIR}/data/projects/*.lua")
		set(TEXTURES_WILDCARD "${PACKAGE_DATA_DIR}/data/*.png")
		set(SCRIPTS_WILDCARD "${PACKAGE_DATA_DIR}/data/scripts/*.lua")

		generate_files_list(${PROJECTS_WILDCARD} "Projects")
		generate_files_list(${TEXTURES_WILDCARD} "Textures")
		generate_files_list(${SCRIPTS_WILDCARD} "Scripts")

		include(custom_fontawesome)
		if(CUSTOM_WITH_FONTAWESOME)
			file(GLOB FONT_FILES "${PACKAGE_DATA_DIR}/data/fonts/*.ttf")
		endif()
		if(EXISTS ${PACKAGE_DATA_DIR}/data/config.lua)
			set(CONFIG_FILE ${PACKAGE_DATA_DIR}/data/config.lua)
		endif()
		file(GLOB PROJECT_FILES ${PROJECTS_WILDCARD})
		file(GLOB TEXTURE_FILES ${TEXTURES_WILDCARD})
		file(GLOB SCRIPT_FILES ${SCRIPTS_WILDCARD})
		set(PACKAGE_ANDROID_ASSETS ${CONFIG_FILE} ${FONT_FILES} ${PROJECT_FILES} ${TEXTURE_FILES} ${SCRIPT_FILES} CACHE STRING "" FORCE)
	endif()

	if(CMAKE_SYSTEM_NAME STREQUAL "Android" AND CUSTOM_WITH_FONTAWESOME)
		target_compile_definitions(${PACKAGE_EXE_NAME} PRIVATE "WITH_FONTAWESOME")
		target_include_directories(${PACKAGE_EXE_NAME} PRIVATE ${GENERATED_INCLUDE_DIR}/../../${ICONFONTCPPHEADERS_SOURCE_DIR_NAME})
	endif()

	# Needed to compile on Android
	set(GENERATED_SOURCES ${GENERATED_SOURCES} PARENT_SCOPE)
endfunction()

function(callback_end)
	if(NOT CMAKE_SYSTEM_NAME STREQUAL "Android" AND IS_DIRECTORY ${PACKAGE_DATA_DIR}/docs)
		get_filename_component(PARENT_DATA_INSTALL_DESTINATION ${DATA_INSTALL_DESTINATION} DIRECTORY)
		if(PARENT_DATA_INSTALL_DESTINATION STREQUAL "")
			set(PARENT_DATA_INSTALL_DESTINATION .)
		endif()
		install(DIRECTORY ${PACKAGE_DATA_DIR}/docs DESTINATION ${PARENT_DATA_INSTALL_DESTINATION} PATTERN "*.adoc" EXCLUDE)
	endif()

	if(CUSTOM_ITCHIO_BUILD AND CMAKE_SYSTEM_NAME STREQUAL "Linux" AND PACKAGE_OPTIONS_PRESETS STREQUAL "BinDist")
		# Set a relative data directory when building for Itch.io on Linux
		file(RELATIVE_PATH PACKAGE_DEFAULT_DATA_DIR_NEW
				${CMAKE_INSTALL_PREFIX}/${RUNTIME_INSTALL_DESTINATION}
				${CMAKE_INSTALL_PREFIX}/${DATA_INSTALL_DESTINATION}) # Always strips trailing slash
		set(PACKAGE_DEFAULT_DATA_DIR_NEW "${PACKAGE_DEFAULT_DATA_DIR_NEW}/")

		get_target_property(COMPILE_DEFS ${PACKAGE_EXE_NAME} COMPILE_DEFINITIONS)
		string(REPLACE ${PACKAGE_DEFAULT_DATA_DIR} ${PACKAGE_DEFAULT_DATA_DIR_NEW} COMPILE_DEFS "${COMPILE_DEFS}")
		set_property(TARGET ${PACKAGE_EXE_NAME} PROPERTY COMPILE_DEFINITIONS ${COMPILE_DEFS})
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
	file(WRITE ${STRINGS_H_FILE} "#ifndef PACKAGE_${TYPE_UPPER}_STRINGS\n")
	file(APPEND ${STRINGS_H_FILE} "#define PACKAGE_${TYPE_UPPER}_STRINGS\n\n")
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

	target_sources(${PACKAGE_EXE_NAME} PRIVATE ${STRINGS_H_FILE} ${STRINGS_CPP_FILE})
	list(APPEND GENERATED_SOURCES ${STRINGS_CPP_FILE})
	set(GENERATED_SOURCES ${GENERATED_SOURCES} PARENT_SCOPE)
endfunction()
