cmake_minimum_required(VERSION 3.7)

get_filename_component(PACKAGE_SOURCE_DIR ${CMAKE_MODULE_PATH} DIRECTORY)
include(package_info)
if(COMMAND callback_start)
	callback_start()
endif()

project(${PACKAGE_NAME}-Android-${ANDROID_ABI})

if(NCINE_DYNAMIC_LIBRARY)
	add_library(ncine SHARED IMPORTED)
	set_target_properties(ncine PROPERTIES
		IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/ncine/${ANDROID_ABI}/libncine.so"
		INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/ncine/include")
else()
	add_library(ncine STATIC IMPORTED)
	set_target_properties(ncine PROPERTIES
		IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/ncine/${ANDROID_ABI}/libncine.a"
		INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/ncine/include")
	target_compile_definitions(ncine INTERFACE "NCINE_STATIC")
endif()

add_library(ncine_main STATIC IMPORTED)
set_target_properties(ncine_main PROPERTIES
	IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/ncine/${ANDROID_ABI}/libncine_main.a"
	INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/ncine/include")

if(COMMAND callback_before_target)
	callback_before_target()
endif()

foreach(SOURCE ${PACKAGE_SOURCES})
	if(IS_ABSOLUTE ${SOURCE})
		list(APPEND ANDROID_PACKAGE_SOURCES ${SOURCE})
	else()
		list(APPEND ANDROID_PACKAGE_SOURCES ${PACKAGE_SOURCE_DIR}/${SOURCE})
	endif()
endforeach()

foreach(INCLUDE_DIR ${PACKAGE_INCLUDE_DIRS})
	if(IS_ABSOLUTE ${INCLUDE_DIR})
		list(APPEND ANDROID_PACKAGE_INCLUDE_DIRS "${INCLUDE_DIR}")
	else()
		list(APPEND ANDROID_PACKAGE_INCLUDE_DIRS "${PACKAGE_SOURCE_DIR}/${INCLUDE_DIR}")
	endif()
endforeach()

add_library(native_app_glue STATIC ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
target_link_libraries(native_app_glue PRIVATE android log)
add_library(${PACKAGE_EXE_NAME} SHARED ${ANDROID_PACKAGE_SOURCES})
set_property(TARGET ${PACKAGE_EXE_NAME} PROPERTY OUTPUT_NAME "game")
target_link_libraries(${PACKAGE_EXE_NAME} PRIVATE native_app_glue ncine_main ncine)
target_include_directories(${PACKAGE_EXE_NAME} PRIVATE ${ANDROID_NDK}/sources/android/native_app_glue)

target_compile_definitions(${PACKAGE_EXE_NAME} PRIVATE "$<$<CONFIG:Debug>:${PACKAGE_UPPER_NAME}_DEBUG>")
target_compile_definitions(${PACKAGE_EXE_NAME} PRIVATE "$<$<CONFIG:Debug>:NCINE_ASSERT_BREAK>")

if(IS_DIRECTORY ${CMAKE_SOURCE_DIR}/ncine/include/tracy)
	target_include_directories(${PACKAGE_EXE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/ncine/include/tracy)
	target_compile_definitions(${PACKAGE_EXE_NAME} PRIVATE "TRACY_ENABLE")
endif()

# Convert strings back to the original lists
string(REPLACE " " ";" GENERATED_SOURCES "${GENERATED_SOURCES}")
string(REPLACE " " ";" ANDROID_GENERATED_FLAGS "${ANDROID_GENERATED_FLAGS}")

target_include_directories(${PACKAGE_EXE_NAME} PRIVATE ${ANDROID_PACKAGE_INCLUDE_DIRS})
target_sources(${PACKAGE_EXE_NAME} PRIVATE ${GENERATED_SOURCES})
if(IS_DIRECTORY ${GENERATED_INCLUDE_DIR})
	target_include_directories(${PACKAGE_EXE_NAME} PRIVATE ${GENERATED_INCLUDE_DIR})
endif()
target_compile_definitions(${PACKAGE_EXE_NAME} PRIVATE ${ANDROID_GENERATED_FLAGS})

if(COMMAND callback_after_target)
	callback_after_target()
endif()

if(NOT NCINE_DYNAMIC_LIBRARY)
	include(package_imported_targets)
	target_link_libraries(${PACKAGE_EXE_NAME} PRIVATE
		${EGL_LIBRARY} ${GLES3_LIBRARY} ${OPENSLES_LIBRARY} ${ZLIB_LIBRARY})
	if(PNG_FOUND)
		target_link_libraries(${PACKAGE_EXE_NAME} PRIVATE PNG::PNG)
	endif()
	if(WEBP_FOUND)
		target_link_libraries(${PACKAGE_EXE_NAME} PRIVATE WebP::WebP)
	endif()
	if(OPENAL_FOUND)
		target_link_libraries(${PACKAGE_EXE_NAME} PRIVATE OpenAL::AL)
	endif()
	if(VORBIS_FOUND)
		target_link_libraries(${PACKAGE_EXE_NAME} PRIVATE Vorbis::Vorbisfile)
	endif()
	if(LUA_FOUND)
		target_link_libraries(${PACKAGE_EXE_NAME} PRIVATE Lua::Lua)
	endif()
endif()

# Export ANativeActivity_onCreate() - https://github.com/android-ndk/ndk/issues/381
set_property(TARGET ${PACKAGE_EXE_NAME} APPEND_STRING PROPERTY LINK_FLAGS "-u ANativeActivity_onCreate")

include(package_strip_binaries)
if(COMMAND callback_end)
	callback_end()
endif()
