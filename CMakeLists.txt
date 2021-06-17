cmake_minimum_required(VERSION 3.0.0)

project(ts_reader VERSION 1.0.0)


# Boost

#set(Boost_DEBUG ON)
#set(Boost_DETAILED_FAILURE_MSG ON)

if(WIN32)
	set(BOOST_ROOT "C:/Libraries/boost_1_75_0")
	set(BOOST_LIBRARY_DIR "C:/Libraries/boost_1_75_0/stage/lib")
	set(Boost_NO_SYSTEM_PATHS true)
endif()

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)

find_package(Boost REQUIRED COMPONENTS program_options)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_libraries(${Boost_LIBRARIES})
endif()


# TS reader

file(GLOB TS_READER_SOURCE_FILES src/*.cpp)
add_executable(ts_reader ${TS_READER_SOURCE_FILES})
target_include_directories(ts_reader PRIVATE inc)
target_link_libraries(ts_reader Boost::program_options)
target_compile_features(ts_reader PRIVATE cxx_std_20)


# TS reader test

file(GLOB TS_READER_TEST_SOURCE_FILES ts_reader_test/src/*.cpp)
add_executable(ts_reader_test ${TS_READER_TEST_SOURCE_FILES})
target_compile_features(ts_reader_test PRIVATE cxx_std_20)
