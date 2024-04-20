cmake_minimum_required(VERSION 3.5)

get_filename_component(projectName Simpli NAME)

message("cmake for ${projectName}")

file(
        GLOB
	source_files
	main.cpp
        src/*
)

set(localLibs
	MaLib
        EvalMaxSAT
)

set(externLibs
        z
)



add_definitions(-DNDEBUG)
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -Wall -O2 -g")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -Wall -O3 -static -lrt -pthread -Wl,--whole-archive -lpthread -Wl,--no-whole-archive")
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -Wall -O0 -g")


if(TARGET ${projectName})
	message("${projectName} is already defined")
else()
	FOREACH (localLib ${localLibs})
		add_subdirectory (lib/${localLib} ${localLib})
	ENDFOREACH(localLib)

	add_executable (
		${projectName}
		${source_files}
	)


	FOREACH (localLib ${localLibs})
		include_directories (lib/${localLib}/src)
	ENDFOREACH(localLib)

	set(LIB_LINK
		${localLibs}
		${externLibs}
	)

	target_link_libraries(${projectName}
	    ${LIB_LINK}
	)


endif()

install (TARGETS ${projectName} DESTINATION bin)

