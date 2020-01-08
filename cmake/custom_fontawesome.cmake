if(CUSTOM_WITH_FONTAWESOME)
	# Download and unpack FontAwesome at configure time
	configure_file(cmake/custom_fontawesome_download.in fontawesome-download/CMakeLists.txt)

	execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
		RESULT_VARIABLE result
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/fontawesome-download
	)
	if(result)
		message(STATUS "CMake step for FontAwesome failed: ${result}")
		set(FONTAWESOME_ERROR TRUE)
	endif()

	execute_process(COMMAND ${CMAKE_COMMAND} --build .
		RESULT_VARIABLE result
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/fontawesome-download
	)
	if(result)
		message(STATUS "Build step for FontAwesome failed: ${result}")
		set(FONTAWESOME_ERROR TRUE)
	endif()

	if(FONTAWESOME_ERROR)
		set(CUSTOM_WITH_FONTAWESOME FALSE)
	else()
		execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${PACKAGE_DATA_DIR}/data/fonts
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
				${CMAKE_BINARY_DIR}/fontawesome-src/webfonts/fa-brands-400.ttf
				${CMAKE_BINARY_DIR}/fontawesome-src/webfonts/fa-regular-400.ttf
				${CMAKE_BINARY_DIR}/fontawesome-src/webfonts/fa-solid-900.ttf
				${PACKAGE_DATA_DIR}/data/fonts)
	endif()
endif()
