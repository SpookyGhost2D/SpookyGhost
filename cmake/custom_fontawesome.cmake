if(CUSTOM_WITH_FONTAWESOME)
	set(FONTAWESOME_VERSION_TAG "master")
	# Download release archive (TRUE) or Git repository (FALSE)
	set(FONTAWESOME_DOWNLOAD_ARCHIVE TRUE)

	if(FONTAWESOME_DOWNLOAD_ARCHIVE)
		# Strip the initial "v" character from the version tag
		string(REGEX MATCH "^v[0-9]" FONTAWESOME_STRIP_VERSION ${FONTAWESOME_VERSION_TAG})
		if(FONTAWESOME_STRIP_VERSION STREQUAL "")
			set(FONTAWESOME_VERSION_TAG_DIR ${FONTAWESOME_VERSION_TAG})
		else()
			string(SUBSTRING ${FONTAWESOME_VERSION_TAG} 1 -1 FONTAWESOME_VERSION_TAG_DIR)
		endif()

		if (IS_DIRECTORY ${CMAKE_BINARY_DIR}/Font-Awesome-${FONTAWESOME_VERSION_TAG_DIR})
			message(STATUS "Font-Awesome release file \"${FONTAWESOME_VERSION_TAG}\" has been already downloaded")
		else()
			file(DOWNLOAD https://github.com/FortAwesome/Font-Awesome/archive/${FONTAWESOME_VERSION_TAG}.tar.gz
				${CMAKE_BINARY_DIR}/${FONTAWESOME_VERSION_TAG}.tar.gz STATUS result)

			list(GET result 0 result_code)
			if(result_code)
				message(WARNING "Cannot download Font-Awesome release file ${FONTAWESOME_VERSION_TAG}")
			else()
				message(STATUS "Downloaded Font-Awesome release file \"${FONTAWESOME_VERSION_TAG}\"")
				file(ARCHIVE_EXTRACT INPUT ${CMAKE_BINARY_DIR}/${FONTAWESOME_VERSION_TAG}.tar.gz DESTINATION ${CMAKE_BINARY_DIR})
				file(REMOVE ${CMAKE_BINARY_DIR}/${FONTAWESOME_VERSION_TAG}.tar.gz)
			endif()
		endif()

		if (IS_DIRECTORY ${CMAKE_BINARY_DIR}/Font-Awesome-${FONTAWESOME_VERSION_TAG_DIR})
			file(COPY ${CMAKE_BINARY_DIR}/Font-Awesome-${FONTAWESOME_VERSION_TAG_DIR}/webfonts/fa-brands-400.ttf
				${CMAKE_BINARY_DIR}/Font-Awesome-${FONTAWESOME_VERSION_TAG_DIR}/webfonts/fa-regular-400.ttf
				${CMAKE_BINARY_DIR}/Font-Awesome-${FONTAWESOME_VERSION_TAG_DIR}/webfonts/fa-solid-900.ttf
				DESTINATION ${NCPROJECT_DATA_DIR}/data/fonts)
		else()
			set(CUSTOM_WITH_FONTAWESOME FALSE)
		endif()
	else()
		# Download Font-Awesome repository at configure time
		configure_file(cmake/custom_fontawesome_download.in fontawesome-download/CMakeLists.txt)

		execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
			RESULT_VARIABLE result
			WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/fontawesome-download
		)
		if(result)
			message(STATUS "CMake step for Font-Awesome failed: ${result}")
			set(FONTAWESOME_ERROR TRUE)
		endif()

		execute_process(COMMAND ${CMAKE_COMMAND} --build .
			RESULT_VARIABLE result
			WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/fontawesome-download
		)
		if(result)
			message(STATUS "Build step for Font-Awesome failed: ${result}")
			set(FONTAWESOME_ERROR TRUE)
		endif()

		if(FONTAWESOME_ERROR)
			set(CUSTOM_WITH_FONTAWESOME FALSE)
		else()
			execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${NCPROJECT_DATA_DIR}/data/fonts
				COMMAND ${CMAKE_COMMAND} -E copy_if_different
					${CMAKE_BINARY_DIR}/fontawesome-src/webfonts/fa-brands-400.ttf
					${CMAKE_BINARY_DIR}/fontawesome-src/webfonts/fa-regular-400.ttf
					${CMAKE_BINARY_DIR}/fontawesome-src/webfonts/fa-solid-900.ttf
					${NCPROJECT_DATA_DIR}/data/fonts)
		endif()
	endif()
endif()
