set_property(GLOBAL PROPERTY USE_FOLDERS ON)
if(UNIX)
	if(IS_DIRECTORY ${NCINE_SHADERS_DIR})
		add_custom_target(symlink_shaders ALL
			COMMAND ${CMAKE_COMMAND} -E create_symlink ${NCINE_SHADERS_DIR} ${PACKAGE_DEFAULT_DATA_DIR}/shaders
			COMMENT "Symlinking shaders..."
		)
		set_target_properties(symlink_shaders PROPERTIES FOLDER "CustomSymlinkTargets")
	endif()
elseif(WIN32)
	if(IS_DIRECTORY ${NCINE_SHADERS_DIR})
		add_custom_target(copy_shaders ALL
			COMMAND ${CMAKE_COMMAND} -E copy_directory ${NCINE_SHADERS_DIR} ${PACKAGE_DEFAULT_DATA_DIR}/shaders
			COMMENT "Copying shaders..."
		)
		set_target_properties(copy_shaders PROPERTIES FOLDER "CustomCopyTargets")
	endif()

	if(NCINE_DYNAMIC_LIBRARY)
		add_custom_target(copy_ncine_dll ALL
			COMMAND ${CMAKE_COMMAND} -E copy_if_different ${NCINE_LOCATION} ${CMAKE_BINARY_DIR}
			COMMENT "Copying nCine DLL..."
		)
		set_target_properties(copy_ncine_dll PROPERTIES FOLDER "CustomCopyTargets")
	endif()
endif()

if(MSVC)
	file(GLOB MSVC_DLL_FILES ${MSVC_BINDIR}/*.dll)

	add_custom_target(copy_dlls ALL
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${MSVC_DLL_FILES} ${CMAKE_BINARY_DIR}
		COMMENT "Copying DLLs..."
	)
	set_target_properties(copy_dlls PROPERTIES FOLDER "CustomCopyTargets")
endif()
