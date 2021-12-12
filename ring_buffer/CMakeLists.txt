cmake_minimum_required(VERSION 3.0.2)
project(test)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/out)

add_subdirectory(ringbuffer)

if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

    set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE}
        CACHE STRING "Debug or tsan" FORCE)

    # cmake -DCMAKE_BUILD_TYPE=tsan
    set(CMAKE_CXX_FLAGS_TSAN
        "-fsanitize=thread -g -O1" CACHE STRING "tsan" FORCE)
endif(UNIX)

add_executable(ringbuffer_test
    test.cpp
    )
target_include_directories(ringbuffer_test PUBLIC ${PROJECT_SOURCE_DIR}/ringbuffer)
target_link_libraries(ringbuffer_test ringbuffer)

add_executable(data_race_test
    ringbuffer_try/dataRaceTest.cpp
    )
target_include_directories(data_race_test PUBLIC ${PROJECT_SOURCE_DIR}/ringbuffer ${PROJECT_SOURCE_DIR}/ring_buffer_try)
target_link_libraries(data_race_test ringbuffer)
