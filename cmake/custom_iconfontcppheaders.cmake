if(CUSTOM_WITH_FONTAWESOME)
	# Download and unpack IconFontCppHeaders at configure time
	configure_file(cmake/custom_iconfontcppheaders_download.in iconfontcppheaders-download/CMakeLists.txt)

	execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
		RESULT_VARIABLE result
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/iconfontcppheaders-download
	)
	if(result)
		message(STATUS "CMake step for IconFontCppHeaders failed: ${result}")
		set(ICONFONTCPPHEADERS_ERROR TRUE)
	endif()

	execute_process(COMMAND ${CMAKE_COMMAND} --build .
		RESULT_VARIABLE result
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/iconfontcppheaders-download
	)
	if(result)
		message(STATUS "Build step for IconFontCppHeaders failed: ${result}")
		set(ICONFONTCPPHEADERS_ERROR TRUE)
	endif()

	if(ICONFONTCPPHEADERS_ERROR)
		set(CUSTOM_WITH_FONTAWESOME FALSE)
	else()
		target_compile_definitions(${PACKAGE_EXE_NAME} PRIVATE "WITH_FONTAWESOME")
		target_include_directories(${PACKAGE_EXE_NAME} PRIVATE ${CMAKE_BINARY_DIR}/iconfontcppheaders-src)
	endif()
endif()
