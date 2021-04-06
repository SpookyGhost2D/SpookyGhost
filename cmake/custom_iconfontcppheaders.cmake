if(CUSTOM_WITH_FONTAWESOME)
	set(ICONFONTCPPHEADERS_VERSION_TAG "main")
	# Download release archive (TRUE) or Git repository (FALSE)
	set(ICONFONTCPPHEADERS_DOWNLOAD_ARCHIVE TRUE)

	if(ICONFONTCPPHEADERS_DOWNLOAD_ARCHIVE)
		# Strip the initial "v" character from the version tag
		string(REGEX MATCH "^v[0-9]" ICONFONTCPPHEADERS_STRIP_VERSION ${ICONFONTCPPHEADERS_VERSION_TAG})
		if(ICONFONTCPPHEADERS_STRIP_VERSION STREQUAL "")
			set(ICONFONTCPPHEADERS_VERSION_TAG_DIR ${ICONFONTCPPHEADERS_VERSION_TAG})
		else()
			string(SUBSTRING ${ICONFONTCPPHEADERS_VERSION_TAG} 1 -1 ICONFONTCPPHEADERS_VERSION_TAG_DIR)
		endif()

		set(ICONFONTCPPHEADERS_SOURCE_DIR_NAME IconFontCppHeaders-${ICONFONTCPPHEADERS_VERSION_TAG_DIR})
	else()
		set(ICONFONTCPPHEADERS_SOURCE_DIR_NAME iconfontcppheaders-src)
	endif()

	if(ANDROID)
		return()
	endif()

	if(ICONFONTCPPHEADERS_DOWNLOAD_ARCHIVE)
		if (IS_DIRECTORY ${CMAKE_BINARY_DIR}/${ICONFONTCPPHEADERS_SOURCE_DIR_NAME})
			message(STATUS "IconFontCppHeaders release file \"${ICONFONTCPPHEADERS_VERSION_TAG}\" has been already downloaded")
		else()
			file(DOWNLOAD https://github.com/juliettef/IconFontCppHeaders/archive/${ICONFONTCPPHEADERS_VERSION_TAG}.tar.gz
				${CMAKE_BINARY_DIR}/${ICONFONTCPPHEADERS_VERSION_TAG}.tar.gz STATUS result)

			list(GET result 0 result_code)
			if(result_code)
				message(WARNING "Cannot download IconFontCppHeaders release file ${ICONFONTCPPHEADERS_VERSION_TAG}")
			else()
				message(STATUS "Downloaded IconFontCppHeaders release file \"${ICONFONTCPPHEADERS_VERSION_TAG}\"")
				file(ARCHIVE_EXTRACT INPUT ${CMAKE_BINARY_DIR}/${ICONFONTCPPHEADERS_VERSION_TAG}.tar.gz DESTINATION ${CMAKE_BINARY_DIR})
				file(REMOVE ${CMAKE_BINARY_DIR}/${ICONFONTCPPHEADERS_VERSION_TAG}.tar.gz)
			endif()
		endif()

		if (IS_DIRECTORY ${CMAKE_BINARY_DIR}/${ICONFONTCPPHEADERS_SOURCE_DIR_NAME})
			target_compile_definitions(${PACKAGE_EXE_NAME} PRIVATE "WITH_FONTAWESOME")
			target_include_directories(${PACKAGE_EXE_NAME} PRIVATE ${CMAKE_BINARY_DIR}/${ICONFONTCPPHEADERS_SOURCE_DIR_NAME})
		else()
			set(CUSTOM_WITH_FONTAWESOME FALSE)
		endif()
	else()
		# Download IconFontCppHeaders repository at configure time
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
			target_include_directories(${PACKAGE_EXE_NAME} PRIVATE ${CMAKE_BINARY_DIR}/${ICONFONTCPPHEADERS_SOURCE_DIR_NAME})
		endif()
	endif()
endif()
