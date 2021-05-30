cmake_minimum_required( VERSION 2.6 )
PROJECT(project_lanarts_net)

set( DEPS ../../dependencies)

include_directories(
	/home/adomurad/sources/emsdk/emscripten/1.38.25/system/include/
	../common-lib/include
	../luawrap-lib/include
	${DEPS}/UnitTest++ 
)

aux_source_directory( "impl/" lanarts_net_src)
aux_source_directory( "tests/" lanarts_net_tests_src)

add_library( lanarts_net
             STATIC
             ${lanarts_net_src})

set(USE_FLAGS "--bind -s MINIFY_HTML=0 -s TOTAL_MEMORY=800MB")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${USE_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${USE_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${USE_FLAGS}")
set(CMAKE_EXECUTABLE_SUFFIX .html)

add_executable( lanarts_net_tests
             ${lanarts_net_tests_src})

target_link_libraries(
	lanarts_net_tests
	lanarts_net
	lcommon
	UnitTest++
)
