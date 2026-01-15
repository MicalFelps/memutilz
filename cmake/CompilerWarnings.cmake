function(memutilz_set_warnings)
	if(MSVC)
		set(_warnings
			/W4
			/permissive-
		)
		if(MEMUTILZ_WARNINGS_AS_ERRORS)
			list(APPEND _warnings /WX)
		endif()
	elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		set(_warnings
			-Wall
			-Wextra
			-Wpedantic
			-Wshadow
			-Wconversion
			-Wsign-conversion
			-Wnon-virtual-dtor
		)
		if(MEMUTILZ_WARNINGS_AS_ERRORS)
			list(APPEND _warnings -Werror)
		endif()
	elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		set(_warnings
			-Wall
			-Wextra
			-Wpedantic
			-Wshadow
			-Wconversion
			-Wsign-conversion
			-Wduplicated-cond
			-Wlogical-op
		)
		if(MEMUTILZ_WARNINGS_AS_ERRORS)
			list(APPEND _warnings -Werror)
		endif()
	else()
		message(STATUS "No warning set defined for ${CMAKE_CXX_COMPILER_ID}")
		return()
	endif()

	target_compile_options(${target} PRIVATE
		$<$<COMPILE_LANGUAGE:CXX>:${_warnings}>
		$<$<COMPILE_LANGUAGE:C>:${_warnings}>
	)
endfunction()