# CMakeList.txt : CMake project for librdkafka-wrapper, include source and define
# project specific logic here.
#
#list all vars
#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

cmake_minimum_required (VERSION 3.8)

project (oekafka-wrapper VERSION 0.1 LANGUAGES C)

# shared source files
set(SOURCE_FILES oekafka-wrapper.c oekafka-wrapper.h oekafka-wrapper.def)

#add_library (oekafka-wrapper-static STATIC ${SOURCE_FILES})
add_library (oekafka-wrapper SHARED ${SOURCE_FILES})



# add consumer test executable
set(CONSUMER_SOURCE_FILES consumer-test.c oekafka-wrapper.h)
add_executable (consumer-test ${CONSUMER_SOURCE_FILES})
target_link_libraries(consumer-test oekafka-wrapper)

# add producer test executable
set(PRODUCER_SOURCE_FILES producer-test.c oekafka-wrapper.h)
add_executable (producer-test ${PRODUCER_SOURCE_FILES})
target_link_libraries(producer-test oekafka-wrapper)

# add combined test executable
set(COMBINED_SOURCE_FILES combined-test.c oekafka-wrapper.h)
add_executable (combined-test ${COMBINED_SOURCE_FILES})
target_link_libraries(combined-test oekafka-wrapper)


if (UNIX)
    message(STATUS "Finding Linux library")
    #find_library(LIBRDKAFKA librdkafka.so PATHS ${CMAKE_CURRENT_SOURCE_DIR}/packages/librdkafka.redist.1.3.0/runtimes/linux-x64/native/)
    find_library(LIBRDKAFKA centos7-librdkafka.so PATHS ${CMAKE_CURRENT_SOURCE_DIR}/packages/librdkafka.redist.1.3.0/runtimes/linux-x64/native/)
    
else()
    message(STATUS "IS NOT UNIX")
    find_library(LIBRDKAFKA librdkafka.lib PATHS ${CMAKE_CURRENT_SOURCE_DIR}/packages/librdkafka.redist.1.3.0/build/native/lib/win/x64/win-x64-Release/v120/)
endif()

if(NOT LIBRDKAFKA)
    message(FATAL_ERROR "LIBRDKAFKA library not found!")
else()
    message(STATUS "Found LIBRDKAFKA: " ${LIBRDKAFKA})
    target_link_libraries(oekafka-wrapper ${LIBRDKAFKA})
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/packages.config
${CMAKE_CURRENT_BINARY_DIR}/packages.config COPYONLY)


install(TARGETS oekafka-wrapper
  DESTINATION bin/
)


install(TARGETS consumer-test
  DESTINATION bin/
)

install(TARGETS producer-test
  DESTINATION bin/
)

install(TARGETS combined-test
  DESTINATION bin/
)

if (UNIX)
    install(DIRECTORY "${PROJECT_SOURCE_DIR}/packages/librdkafka.redist.1.3.0/runtimes/linux-x64/native/"
            DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
else()
    install(DIRECTORY "${PROJECT_SOURCE_DIR}/packages/librdkafka.redist.1.3.0/runtimes/win-x64/native/"
            DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
endif()
